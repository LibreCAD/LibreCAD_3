"""Own renderer test/build commands without sweeping unrelated processes.

Linux requires a fresh single-purpose CLI process: subreaper adoption and pidfds
cover descendants that double-fork or leave their group. CPU commands on macOS
pin the root until its private group is retired; escaped sessions are not covered.
Windows assigns a suspended root to a kill-on-close Job before resuming it.
These are ownership/cleanup boundaries, not security sandboxes.
"""

import ctypes
import json
import errno
import math
import os
import re
import signal
import stat
import subprocess
import sys
import time
from pathlib import Path

from validate_renderer_inventory import sha256


def children():
    return sorted({int(pid) for task in Path('/proc/self/task').iterdir()
                   for pid in (task / 'children').read_text().split()})


def identity(pid):
    text = Path('/proc/{}/stat'.format(pid)).read_text()
    end = text.rindex(')')
    fields = text[end + 2:].split()
    return {'pid': pid, 'start_ticks': int(fields[19]), 'parent': int(fields[1]),
            'state': fields[0], 'name': text[text.index('(') + 1:end]}


def subreaper(value=None):
    libc = ctypes.CDLL(None, use_errno=True)
    previous = ctypes.c_int()
    if libc.prctl(37, ctypes.byref(previous), 0, 0, 0) != 0: # PR_GET_CHILD_SUBREAPER
        raise OSError(ctypes.get_errno(), 'Cannot query Linux child subreaper')
    if value is not None and libc.prctl(36, ctypes.c_ulong(value), 0, 0, 0) != 0:
        raise OSError(ctypes.get_errno(), 'Cannot set Linux child subreaper')
    return previous.value


def capture_display(env):
    display = env.get('DISPLAY', '')
    if not re.fullmatch(r':\d+', display):
        raise ValueError('Supervisor worker requires its own numbered Xvfb display')
    number = display[1:]
    lock, socket = Path('/tmp/.X' + number + '-lock'), Path('/tmp/.X11-unix/X' + number)
    pid = int(lock.read_text().strip())
    server = identity(pid)
    executable = Path('/proc/{}/exe'.format(pid)).resolve(strict=True)
    command = Path('/proc/{}/cmdline'.format(pid)).read_bytes().split(b'\0')
    auth = Path(env.get('XAUTHORITY', '')).resolve(strict=True)
    if (server['name'] != 'Xvfb' or executable.name != 'Xvfb' or display.encode() not in command or
            not auth.is_file() or auth.parent.parent != Path('/tmp').resolve() or
            not auth.parent.name.startswith('lc-xvfb-') or stat.S_IMODE(auth.parent.stat().st_mode) != 0o700 or
            stat.S_IMODE(auth.stat().st_mode) & 0o077):
        raise ValueError('Worker display/authentication is not the owned private Xvfb session')
    files = []
    for path in (lock, socket):
        info = path.lstat()
        if info.st_uid != os.getuid() or (path == socket and not stat.S_ISSOCK(info.st_mode)):
            raise ValueError('Unexpected Xvfb socket ownership/type')
        files.append({'path': str(path), 'device': info.st_dev, 'inode': info.st_ino})
    return {'display': display, 'server': server, 'executable': str(executable),
            'executable_sha256': sha256(executable.read_bytes()), 'private_auth': True, 'files': files}


def validate_display_retired(display, scope):
    if not isinstance(display, dict) or not isinstance(display.get('display'), str) or not re.fullmatch(r':\d+', display['display']):
        raise ValueError('Missing owned Xvfb display identity')
    server = display.get('server', {})
    digest = display.get('executable_sha256')
    if (not isinstance(server, dict) or type(server.get('pid')) is not int or server['pid'] <= 0 or
            type(server.get('start_ticks')) is not int or server['start_ticks'] <= 0 or server.get('name') != 'Xvfb' or
            server.get('parent') != scope.get('launcher', {}).get('pid') or
            display.get('private_auth') is not True or
            not isinstance(digest, str) or not re.fullmatch(r'[0-9a-f]{64}', digest) or
            scope.get('inputs_sha256', {}).get(display.get('executable')) != display.get('executable_sha256')):
        raise ValueError('Xvfb provider/launcher ownership mismatch')
    number = display['display'][1:]
    expected = {'/tmp/.X' + number + '-lock', '/tmp/.X11-unix/X' + number}
    files = display.get('files')
    if not isinstance(files, list) or len(files) != 2 or any(not isinstance(item, dict) for item in files) or {item.get('path') for item in files} != expected:
        raise ValueError('Invalid Xvfb lock/socket manifest')
    for item in files:
        if any(type(item.get(key)) is not int or item[key] < 0 for key in ('device', 'inode')):
            raise ValueError('Invalid Xvfb file identity')
        try:
            current = Path(item['path']).lstat()
        except FileNotFoundError:
            continue
        if (current.st_dev, current.st_ino) == (item['device'], item['inode']):
            raise ValueError('Owned Xvfb lock/socket remains after process cleanup')
        # A new, unrelated server may already have reused the display number.
        # Never remove or signal it; our captured file identities have retired.


def _signal_child(observed, signum):
    pid = observed['pid']
    try:
        fd = os.pidfd_open(pid)
    except ProcessLookupError:
        return False
    try:
        current = identity(pid)
        if current['start_ticks'] != observed['start_ticks'] or current['parent'] != os.getpid():
            raise ValueError('Process identity/ownership changed before cleanup')
        signal.pidfd_send_signal(fd, signum)
        return True
    except ProcessLookupError:
        return False
    finally:
        os.close(fd)


def _retire(proc, term_grace, kill_grace):
    started = time.monotonic()
    reaped, sent, signaled = [], set(), []
    signum, deadline = signal.SIGTERM, None
    while True:
        no_children = False
        for _ in range(64): # Check deadlines/signals even if children keep exiting.
            try:
                event = os.waitid(os.P_ALL, 0, os.WEXITED | os.WNOHANG | os.WNOWAIT)
            except ChildProcessError:
                no_children = True
                break
            if event is None:
                break
            observed = identity(event.si_pid) # WNOWAIT pins the zombie until waitpid.
            pid, status = os.waitpid(event.si_pid, 0)
            code = os.waitstatus_to_exitcode(status)
            reaped.append({**observed, 'exit_status': code})
            if proc is not None and pid == proc.pid:
                proc.returncode = code
        remaining = []
        for pid in children():
            try:
                remaining.append(identity(pid))
            except FileNotFoundError:
                pass # Exited between enumeration and inspection; re-check waitid.
        if no_children and not remaining:
            return {'all_children_reaped': True, 'remaining': [], 'reaped_at_cleanup': reaped,
                    'signals': signaled, 'cleanup_seconds': time.monotonic() - started}
        now = time.monotonic()
        if deadline is not None and now >= deadline and signum == signal.SIGTERM:
            signum, deadline = signal.SIGKILL, None
        for observed in remaining:
            key = (observed['pid'], observed['start_ticks'], signum)
            if observed['state'] != 'Z' and key not in sent:
                if _signal_child(observed, signum):
                    signaled.append({**observed, 'signal': int(signum)})
                sent.add(key)
        # A scheduling delay before signaling cannot consume the signal's grace.
        # Newly adopted children share the current phase; they never reset it.
        if deadline is None:
            deadline = time.monotonic() + (term_grace if signum == signal.SIGTERM else kill_grace)
        elif now >= deadline:
            return {'all_children_reaped': False, 'remaining': remaining, 'reaped_at_cleanup': reaped,
                    'signals': signaled, 'cleanup_seconds': time.monotonic() - started}
        time.sleep(0.01)


def supervise(command, directory, timeout, env=None, tools=(), term_grace=2, kill_grace=3):
    """Return explicit launcher/cleanup results; never turn a failed child into success."""
    if sys.platform != 'linux' or not all(hasattr(os, name) for name in ('pidfd_open', 'waitid', 'WNOWAIT')) or not hasattr(signal, 'pidfd_send_signal'):
        raise ValueError('Isolated renderer supervision requires Linux pidfd/waitid support')
    if not 0 < timeout <= 360 or not 0 < term_grace <= 5 or not 0 < kill_grace <= 5:
        raise ValueError('Invalid bounded supervisor timeout')
    if signal.getsignal(signal.SIGCHLD) != signal.SIG_DFL:
        raise ValueError('Supervisor requires default SIGCHLD handling to preserve exit status')
    if len(list(Path('/proc/self/task').iterdir())) != 1:
        raise ValueError('Supervisor requires a dedicated single-threaded CLI process')
    if children():
        raise ValueError('Supervisor requires a fresh child-free CLI process')
    inputs = {str(path.resolve(strict=True)): sha256(path.read_bytes()) for path in tools}
    record = {'schema_version': 1, 'evidence_kind': 'r0-process-scope', 'root_pid': os.getpid(),
              'fresh_child_free_parent': True, 'subreaper': True, 'pidfd_signals': True,
              'command': command, 'inputs_sha256': inputs, 'timeout_seconds': timeout,
              'term_grace_seconds': term_grace, 'kill_grace_seconds': kill_grace,
              'launcher_exit_status': None, 'launcher_timed_out': False, 'interrupted_signal': None,
              'all_children_reaped': False, 'errors': []}
    proc, previous, handlers = None, subreaper(1), {}

    def interrupted(signum, _frame):
        record['interrupted_signal'] = signum
        raise InterruptedError('Renderer supervisor interrupted by signal ' + str(signum))

    try:
        for signum in (signal.SIGINT, signal.SIGTERM):
            handlers[signum] = signal.signal(signum, interrupted)
        with (directory / 'launcher.stdout.log').open('wb') as out, (directory / 'launcher.stderr.log').open('wb') as err:
            proc = subprocess.Popen(command, env=env, stdin=subprocess.DEVNULL, stdout=out, stderr=err,
                                    start_new_session=True)
            record['launcher'] = identity(proc.pid)
            try:
                proc.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                record['launcher_timed_out'] = True
    except BaseException as error:
        record['errors'].append(type(error).__name__ + ': ' + str(error))
    finally:
        # Finish bounded cleanup even if another interrupt arrives. Restore the
        # caller's handlers only after every adopted child has been waited for.
        for signum in handlers:
            signal.signal(signum, signal.SIG_IGN)
        try:
            record.update(_retire(proc, term_grace, kill_grace))
        except BaseException as error:
            record['errors'].append('cleanup: ' + type(error).__name__ + ': ' + str(error))
        if proc is not None:
            record['launcher_exit_status'] = proc.returncode
        try:
            if {name: sha256(Path(name).read_bytes()) for name in inputs} != inputs:
                record['errors'].append('Supervisor tool inputs changed during execution')
        except BaseException as error:
            record['errors'].append('provider: ' + type(error).__name__ + ': ' + str(error))
        finally:
            try:
                subreaper(previous)
                record['subreaper_restored'] = subreaper() == previous
            except OSError as error:
                record['errors'].append('restore: ' + str(error))
            for signum, handler in handlers.items():
                signal.signal(signum, handler)
    return record


def validate_scope(record, display_required=True, expected_exit=0):
    if type(display_required) is not bool or type(expected_exit) is not int or expected_exit not in (0,1):
        raise ValueError('Invalid process scope policy')
    if (not isinstance(record, dict) or type(record.get('schema_version')) is not int or record['schema_version'] != 1 or
            record.get('evidence_kind') != 'r0-process-scope' or
            any(record.get(key) is not True for key in ('fresh_child_free_parent', 'subreaper', 'subreaper_restored', 'pidfd_signals', 'all_children_reaped')) or
            record.get('remaining') != [] or record.get('errors') != [] or
            type(record.get('launcher_exit_status')) is not int or record['launcher_exit_status'] != expected_exit or
            record.get('launcher_timed_out') is not False or record.get('interrupted_signal') is not None):
        raise ValueError('Renderer launcher/descendant cleanup failed or is incomplete')
    if display_required:
        if record.get('runtime_directory_removed') is not True or record.get('display_files_retired') is not True:
            raise ValueError('Renderer display cleanup is incomplete')
    elif (record.get('display_required') is not False or any(record.get(k) is not None for k in
            ('display','runtime_directory_removed','display_files_retired'))):
        raise ValueError('Non-display command cannot claim display retirement')
    root = record.get('root_pid')
    duration = record.get('cleanup_seconds')
    if (type(root) is not int or root <= 0 or type(duration) not in (int, float) or
            not math.isfinite(duration) or duration < 0 or
            not isinstance(record.get('reaped_at_cleanup'), list) or not isinstance(record.get('signals'), list)):
        raise ValueError('Invalid process-scope identity/timing/retirement metadata')
    seen = set()
    for field in ('reaped_at_cleanup', 'signals'):
        for item in record[field]:
            if (not isinstance(item, dict) or type(item.get('pid')) is not int or item['pid'] <= 0 or
                    type(item.get('start_ticks')) is not int or item['start_ticks'] <= 0 or
                    item.get('parent') != root or not isinstance(item.get('name'), str) or not item['name']):
                raise ValueError('Invalid retired process identity')
            key = (item['pid'], item['start_ticks'])
            if field == 'reaped_at_cleanup':
                if key in seen or type(item.get('exit_status')) is not int:
                    raise ValueError('Invalid/duplicate reaped process')
                seen.add(key)
            elif type(item.get('signal')) is not int or item['signal'] not in (15, 9): # Linux record; portable reader.
                raise ValueError('Invalid cleanup signal')


def _mac_group(pid):
    lib=ctypes.CDLL('/usr/lib/libproc.dylib',use_errno=True)
    lib.proc_listpgrppids.argtypes=[ctypes.c_int,ctypes.c_void_p,ctypes.c_int]
    lib.proc_listpgrppids.restype=ctypes.c_int
    ctypes.set_errno(0)
    count=lib.proc_listpgrppids(pid,None,0)
    if count==0 and ctypes.get_errno() not in (0,errno.ESRCH):raise OSError(ctypes.get_errno(),'Cannot query owned group')
    if count<0 or count>65536:raise OSError('Cannot bound owned process-group membership')
    # This named wrapper returns a PID count (unlike proc_listpids' byte count).
    values=(ctypes.c_int*(count+64))()
    ctypes.set_errno(0)
    size=lib.proc_listpgrppids(pid,values,ctypes.sizeof(values))
    if size==0 and ctypes.get_errno() not in (0,errno.ESRCH):raise OSError(ctypes.get_errno(),'Cannot query owned group')
    if size<0 or size>=len(values):
        raise OSError('Cannot enumerate complete owned process group')
    return [p for p in values[:size] if p]


class _DarwinSiginfo(ctypes.Structure):
    # Darwin LP64 siginfo_t; si_value is an int/pointer union (pointer layout).
    _fields_=[('signo',ctypes.c_int32),('error',ctypes.c_int32),('code',ctypes.c_int32),
              ('pid',ctypes.c_int32),('uid',ctypes.c_uint32),('status',ctypes.c_int32),
              ('addr',ctypes.c_void_p),('value',ctypes.c_void_p),('band',ctypes.c_int64),
              ('padding',ctypes.c_uint64*7)]


def _mac_waitid(pid):
    """Observe our unreaped child; CPython exposed Darwin waitid only in 3.13."""
    if sys.platform!='darwin':raise ValueError('Darwin wait observation requires macOS')
    options=os.WEXITED|os.WNOHANG|os.WNOWAIT
    native=getattr(os,'waitid',None)
    if native is not None:
        event=native(os.P_PID,pid,options);observed=event.si_pid if event is not None else 0
    else:
        if ctypes.sizeof(ctypes.c_void_p)!=8 or ctypes.sizeof(_DarwinSiginfo)!=104:
            raise ValueError('Unsupported Darwin siginfo ABI')
        waitid=ctypes.CDLL(None,use_errno=True).waitid
        waitid.argtypes=[ctypes.c_int,ctypes.c_uint32,ctypes.POINTER(_DarwinSiginfo),ctypes.c_int]
        waitid.restype=ctypes.c_int
        info=_DarwinSiginfo();ctypes.set_errno(0)
        if waitid(os.P_PID,pid,ctypes.byref(info),options)!=0:
            raise OSError(ctypes.get_errno(),'Darwin waitid failed')
        observed=info.pid
    if observed not in (0,pid):raise ValueError('Darwin waitid returned a foreign child')
    return observed or None


def _posix_command(command, directory, timeout, env):
    """Pin the root zombie until its private process group has been signaled."""
    with (directory/'stdout.log').open('wb') as out, (directory/'stderr.log').open('wb') as err:
        deadline = time.monotonic()+timeout
        # Even a trivial executable may exit before getpgid/getsid can inspect
        # it on Darwin. Block in an isolated bootstrap until ownership is
        # verified, then exec the real command without changing PID/session.
        bootstrap=('import os,sys,signal\nfd=int(sys.argv[1])\n'
            'if os.read(fd,1)!=b"1": os._exit(126)\n'
            'os.close(fd)\n'
            'for name in ("SIGPIPE","SIGXFZ","SIGXFSZ"):\n'
            ' if hasattr(signal,name): signal.signal(getattr(signal,name),signal.SIG_DFL)\n'
            'try: os.execvpe(sys.argv[2],sys.argv[2:],os.environ)\n'
            'except OSError as error:\n sys.stderr.write(str(error)+"\\n"); os._exit(126)\n')
        reader,writer=os.pipe()
        try:
            proc = subprocess.Popen([sys.executable,'-I','-S','-B','-c',bootstrap,str(reader),*command],
                env=env,stdin=subprocess.DEVNULL,stdout=out,stderr=err,start_new_session=True,pass_fds=(reader,))
        except BaseException:
            os.close(writer);raise
        finally:os.close(reader)
        timed_out = False
        event=None
        private=False
        try:
            private=os.getpgid(proc.pid)==proc.pid and os.getsid(proc.pid)==proc.pid
            if not private:
                raise ValueError('Build command did not create its private group/session')
            if os.write(writer,b'1')!=1:raise OSError('Command ownership handshake failed')
            os.close(writer);writer=None
            while True:
                event=_mac_waitid(proc.pid)
                if event is not None:break
                if time.monotonic() >= deadline:
                    timed_out = True; break
                time.sleep(0.01)
        finally:
            try:
                if writer is not None:os.close(writer)
            finally:
                # No poll()/wait() has reaped the root: its PID/group cannot be
                # reused while we signal only this command's session/group.
                try:
                    if not private:
                        proc.terminate() # Only our unreaped root, never an unverified process group.
                        raise ValueError('Private process-group ownership was not established')
                    members=_mac_group(proc.pid)
                    # Darwin can reject killpg for a group containing only its
                    # exited root. No signal is needed when no descendants remain.
                    if event is None or any(p!=proc.pid for p in members):
                        for sig in (signal.SIGTERM,signal.SIGKILL):
                            try: os.killpg(proc.pid,sig)
                            except ProcessLookupError: pass
                            except PermissionError:
                                exited=_mac_waitid(proc.pid)
                                if exited is not None and set(_mac_group(proc.pid))<={proc.pid}:break
                                raise
                finally:proc.wait(timeout=5)
        deadline = time.monotonic()+2
        retired = False
        while time.monotonic() < deadline:
            if not _mac_group(proc.pid):retired=True;break
            time.sleep(0.01)
    return dict(evidence_kind='renderer-posix-command',launcher_exit_status=proc.returncode,
                launcher_timed_out=timed_out,process_group_retired=retired,root_pid=proc.pid,private_session=private)


def _windows_command(command, directory, timeout, env):
    """Start suspended, assign our kill-on-close Job, then resume; no taskkill race."""
    from ctypes import wintypes as w
    import msvcrt
    class Startup(ctypes.Structure):
        _fields_=[('cb',w.DWORD),('reserved',w.LPWSTR),('desktop',w.LPWSTR),('title',w.LPWSTR),
            ('x',w.DWORD),('y',w.DWORD),('xs',w.DWORD),('ys',w.DWORD),('xc',w.DWORD),('yc',w.DWORD),
            ('fill',w.DWORD),('flags',w.DWORD),('show',w.WORD),('reserved_size',w.WORD),
            ('reserved_bytes',ctypes.c_void_p),('stdin',w.HANDLE),('stdout',w.HANDLE),('stderr',w.HANDLE)]
    class Process(ctypes.Structure):
        _fields_=[('process',w.HANDLE),('thread',w.HANDLE),('pid',w.DWORD),('tid',w.DWORD)]
    class BasicLimit(ctypes.Structure):
        _fields_=[('process_time',ctypes.c_int64),('job_time',ctypes.c_int64),('flags',w.DWORD),
            ('minimum',ctypes.c_size_t),('maximum',ctypes.c_size_t),('active',w.DWORD),
            ('affinity',ctypes.c_size_t),('priority',w.DWORD),('scheduling',w.DWORD)]
    class Limits(ctypes.Structure):
        _fields_=[('basic',BasicLimit),('io',ctypes.c_uint64*6),('memory',ctypes.c_size_t*4)]
    class Accounting(ctypes.Structure):
        _fields_=[('times',ctypes.c_int64*4),('faults',w.DWORD),('total',w.DWORD),
                  ('active',w.DWORD),('terminated',w.DWORD)]
    k=ctypes.WinDLL('kernel32',use_last_error=True)
    signatures={
        'CreateJobObjectW':([ctypes.c_void_p,w.LPCWSTR],w.HANDLE),
        'SetInformationJobObject':([w.HANDLE,ctypes.c_int,ctypes.c_void_p,w.DWORD],w.BOOL),
        'CreateProcessW':([w.LPCWSTR,w.LPWSTR,ctypes.c_void_p,ctypes.c_void_p,w.BOOL,w.DWORD,
                           ctypes.c_void_p,w.LPCWSTR,ctypes.POINTER(Startup),ctypes.POINTER(Process)],w.BOOL),
        'AssignProcessToJobObject':([w.HANDLE,w.HANDLE],w.BOOL),
        'ResumeThread':([w.HANDLE],w.DWORD), 'WaitForSingleObject':([w.HANDLE,w.DWORD],w.DWORD),
        'GetExitCodeProcess':([w.HANDLE,ctypes.POINTER(w.DWORD)],w.BOOL),
        'TerminateJobObject':([w.HANDLE,w.UINT],w.BOOL), 'TerminateProcess':([w.HANDLE,w.UINT],w.BOOL),
        'QueryInformationJobObject':([w.HANDLE,ctypes.c_int,ctypes.c_void_p,w.DWORD,ctypes.c_void_p],w.BOOL),
        'CloseHandle':([w.HANDLE],w.BOOL)}
    for name,(args,result) in signatures.items():
        getattr(k,name).argtypes=args;getattr(k,name).restype=result
    def require(ok):
        if not ok: raise ctypes.WinError(ctypes.get_last_error())
    job=k.CreateJobObjectW(None,None);require(job)
    proc=Process();assigned=False
    try:
        limits=Limits();limits.basic.flags=0x2000 # JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
        require(k.SetInformationJobObject(job,9,ctypes.byref(limits),ctypes.sizeof(limits)))
        with open(os.devnull,'rb') as inp,(directory/'stdout.log').open('wb') as out,(directory/'stderr.log').open('wb') as err:
            for stream in (inp,out,err):os.set_inheritable(stream.fileno(),True)
            startup=Startup();startup.cb=ctypes.sizeof(startup);startup.flags=0x100 # STARTF_USESTDHANDLES
            startup.stdin=msvcrt.get_osfhandle(inp.fileno());startup.stdout=msvcrt.get_osfhandle(out.fileno())
            startup.stderr=msvcrt.get_osfhandle(err.fileno())
            environment=ctypes.create_unicode_buffer('\0'.join(k+'='+v for k,v in
                sorted((os.environ if env is None else env).items(),key=lambda item:item[0].casefold()))+'\0\0')
            argv=ctypes.create_unicode_buffer(subprocess.list2cmdline(command))
            require(k.CreateProcessW(None,argv,None,None,True,0x4|0x400,environment,None,ctypes.byref(startup),ctypes.byref(proc)))
            require(k.AssignProcessToJobObject(job,proc.process));assigned=True
            require(k.ResumeThread(proc.thread)!=0xffffffff)
            status=k.WaitForSingleObject(proc.process,int(timeout*1000))
            require(status in (0,258));timed_out=status==258
            code=w.DWORD();require(k.GetExitCodeProcess(proc.process,ctypes.byref(code)))
            require(k.TerminateJobObject(job,1)) # Retire any leftover descendants as well as a timed-out root.
            retired=False;deadline=time.monotonic()+3
            while time.monotonic()<deadline:
                accounting=Accounting()
                require(k.QueryInformationJobObject(job,1,ctypes.byref(accounting),ctypes.sizeof(accounting),None))
                if accounting.active==0:retired=True;break
                time.sleep(0.01)
            return dict(evidence_kind='renderer-windows-job',launcher_exit_status=int(code.value),
                        launcher_timed_out=timed_out,all_children_retired=retired)
    finally:
        if proc.process and not assigned:
            k.TerminateProcess(proc.process,1) # Never resumed outside our Job.
            k.WaitForSingleObject(proc.process,3000)
        k.CloseHandle(job) # kill-on-close also covers exceptional paths after assignment.
        for handle in (proc.thread,proc.process):
            if handle:k.CloseHandle(handle)


def run_command(command, directory, timeout, env=None, expected_exit=0):
    """Owned source-build command, with raw logs and explicit expected rejection."""
    if type(timeout) not in (int,float) or not 0<timeout<=300 or type(expected_exit) is not int or expected_exit not in (0,1):
        raise ValueError('Invalid command deadline/expected exit')
    executable=Path(command[0]).resolve(strict=True)
    command=[str(executable),*command[1:]]
    before=sha256(executable.read_bytes())
    try:
        if sys.platform=='linux':
            record=supervise(command,directory,timeout,env,(executable,))
            record['display_required']=False
            for a,b in (('launcher.stdout.log','stdout.log'),('launcher.stderr.log','stderr.log')):
                (directory/a).replace(directory/b)
        elif sys.platform=='win32':record=_windows_command(command,directory,timeout,env)
        elif sys.platform=='darwin':record=_posix_command(command,directory,timeout,env)
        else:raise ValueError('Unsupported source-build execution platform')
    except BaseException as error:
        # A helper exception is not verified retirement. Retain the failure and
        # raw logs rather than publishing a partial record as successful cleanup.
        record=dict(evidence_kind='renderer-command-incomplete',command=command,executable_sha256=before,
                    expected_exit_status=expected_exit,cleanup_verified=False,error=type(error).__name__+': '+str(error))
        (directory/'processes.json').write_text(json.dumps(record,indent=2)+'\n')
        raise
    record.update(expected_exit_status=expected_exit,command=command,executable_sha256=before)
    (directory/'processes.json').write_text(json.dumps(record,indent=2)+'\n')
    if sys.platform=='linux':validate_scope(record,display_required=False,expected_exit=expected_exit)
    elif (record['launcher_exit_status']!=expected_exit or record['launcher_timed_out'] is not False or
          record['process_group_retired' if sys.platform=='darwin' else 'all_children_retired'] is not True):
        raise ValueError('Source-build command failed, timed out or left its owned scope; logs in '+str(directory))
    if sha256(executable.read_bytes())!=before:raise ValueError('Build executable changed during execution')
    return record
