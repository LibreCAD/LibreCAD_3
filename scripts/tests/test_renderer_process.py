"""CPU-only supervisor contracts; real fork/adoption checks run on Linux only."""

import argparse
import copy
from contextlib import nullcontext
import ctypes
import errno
import json
import os
import shutil
import signal
import subprocess
import sys
import sysconfig
import tempfile
import textwrap
import threading
import time
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

SCRIPTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPTS))
import renderer_process as processes
import run_renderer_checks as runner

TEMP_DIRECTORY = tempfile.TemporaryDirectory


def good_scope():
    return {'schema_version': 1, 'evidence_kind': 'r0-process-scope', 'root_pid': os.getpid(),
            'fresh_child_free_parent': True, 'subreaper': True, 'subreaper_restored': True, 'pidfd_signals': True,
            'all_children_reaped': True, 'remaining': [], 'errors': [], 'launcher_exit_status': 0,
            'launcher_timed_out': False, 'interrupted_signal': None, 'runtime_directory_removed': True, 'display_files_retired': True,
            'cleanup_seconds': 0.01, 'reaped_at_cleanup': [], 'signals': []}


class ProcessContractTests(unittest.TestCase):
    def test_requires_exact_success_and_complete_retirement(self):
        processes.validate_scope(good_scope())
        for key, value in (('schema_version', True), ('launcher_exit_status', 7), ('launcher_exit_status', True),
                           ('launcher_timed_out', True), ('remaining', [{'pid': 42}]),
                           ('errors', ['cleanup failed']), ('all_children_reaped', 1),
                           ('runtime_directory_removed', False), ('interrupted_signal', 15),
                           ('display_files_retired', False),
                           ('subreaper_restored', False),
                           ('root_pid', True), ('cleanup_seconds', float('nan')),
                           ('reaped_at_cleanup', None), ('signals', None)):
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                processes.validate_scope({**good_scope(), key: value})

    def test_retirement_identities_cannot_be_missing_repeated_or_foreign(self):
        item = {'pid': 42, 'start_ticks': 123, 'parent': os.getpid(), 'name': 'owned-fixture',
                'exit_status': 0, 'signal': 15}
        processes.validate_scope({**good_scope(), 'reaped_at_cleanup': [item], 'signals': [item]})
        for item_change in ({'start_ticks': True}, {'parent': -1}, {'pid': 0}, {'name': ''}, {'exit_status': True}):
            with self.subTest(item_change=item_change), self.assertRaises(ValueError):
                processes.validate_scope({**good_scope(), 'reaped_at_cleanup': [{**item, **item_change}]})
        with self.assertRaises(ValueError):
            processes.validate_scope({**good_scope(), 'reaped_at_cleanup': [item, item]})
        with self.assertRaises(ValueError):
            processes.validate_scope({**good_scope(), 'signals': [{**item, 'signal': 999}]})

    def test_reused_or_no_longer_owned_pid_is_never_signaled(self):
        observed = {'pid': 42, 'start_ticks': 123, 'parent': os.getpid()}
        for changed in ({'start_ticks': 124}, {'parent': -1}):
            with patch.object(processes.os, 'pidfd_open', return_value=99, create=True), \
                 patch.object(processes.os, 'close') as close, \
                 patch.object(processes, 'identity', return_value={**observed, **changed}), \
                 patch.object(processes.signal, 'pidfd_send_signal', create=True) as send, \
                 self.assertRaisesRegex(ValueError, 'identity/ownership'):
                processes._signal_child(observed, signal.SIGTERM)
            send.assert_not_called()
            close.assert_called_once_with(99)

    def test_launcher_option_removal_preserves_every_other_argument(self):
        tail = ['--app', '/path with spaces/app', '--background=transparent']
        for argv in (['--xvfb-run', '/path with spaces/xvfb-run', *tail],
                     [*tail, '--xvfb-run=/path with spaces/xvfb-run']):
            self.assertEqual(tail, runner.without_launcher(argv))
        with self.assertRaises(ValueError):
            runner.without_launcher(['--xvfb-run'])

    def test_display_manifest_rejects_live_owned_files_and_preserves_replacements(self):
        scope = {'launcher': {'pid': 100}, 'inputs_sha256': {'/fixture/Xvfb': 'a' * 64}}
        display = {'display': ':9999', 'private_auth': True, 'executable': '/fixture/Xvfb',
                   'executable_sha256': 'a' * 64, 'server': {'pid': 101, 'parent': 100, 'start_ticks': 123, 'name': 'Xvfb'},
                   'files': [{'path': '/tmp/.X9999-lock', 'device': 1, 'inode': 2},
                             {'path': '/tmp/.X11-unix/X9999', 'device': 1, 'inode': 3}]}
        with patch.object(Path, 'lstat', side_effect=FileNotFoundError):
            processes.validate_display_retired(display, scope)
            for change in ({'private_auth': False}, {'server': None}, {'files': []},
                           {'executable_sha256': None}, {'display': ':0;bad'}):
                with self.subTest(change=change), self.assertRaises(ValueError):
                    processes.validate_display_retired({**display, **change}, scope)
        info = argparse.Namespace(st_dev=1, st_ino=2)
        with patch.object(Path, 'lstat', return_value=info), self.assertRaisesRegex(ValueError, 'remains'):
            processes.validate_display_retired(display, scope)
        info.st_ino = 999 # Another server reused the number, not our files.
        with patch.object(Path, 'lstat', return_value=info), patch.object(Path, 'unlink') as unlink:
            processes.validate_display_retired(display, scope)
        unlink.assert_not_called()


class RetirementPhaseTests(unittest.TestCase):
    """Synthetic clock/ownership protocol; no native processes are launched."""
    def model(self, times, stuck=False, adopted=False):
        clock=iter(times);current=[None];children={42:False};signals=[];polls=[0]
        proc=SimpleNamespace(pid=42,returncode=None)
        def now():current[0]=next(clock);return current[0]
        def identity(pid):return dict(pid=pid,start_ticks=pid+100,parent=os.getpid(),
            state='Z' if children[pid] else 'S',name='synthetic-owned-child')
        def waitid(*_):
            if not children:raise ChildProcessError()
            return next((SimpleNamespace(si_pid=p) for p,dead in children.items() if dead),None)
        def waitpid(pid,_):
            self.assertTrue(children.pop(pid));return pid,9 # Synthetic Linux wait status.
        def owned():
            polls[0]+=1
            if adopted and polls[0]==3:children[43]=False
            return sorted(children)
        def send(observed,signum):
            signals.append((observed['pid'],signum,current[0]))
            if signum==9 and not stuck:children[observed['pid']]=True
            return True
        with patch.object(processes,'time',SimpleNamespace(monotonic=now,sleep=lambda _:None)), \
             patch.object(processes,'os',SimpleNamespace(P_ALL=0,WEXITED=1,WNOHANG=2,WNOWAIT=4,
                 waitid=waitid,waitpid=waitpid,waitstatus_to_exitcode=lambda status:-status)), \
             patch.object(processes,'signal',SimpleNamespace(SIGTERM=15,SIGKILL=9)), \
             patch.object(processes,'children',side_effect=owned),patch.object(processes,'identity',side_effect=identity), \
             patch.object(processes,'_signal_child',side_effect=send):
            result=processes._retire(proc,2,3)
        return result,proc,signals

    def test_delay_before_first_signal_cannot_consume_either_phase(self):
        result,proc,signals=self.model([0,7,7,7.1,9.1,9.1,9.2])
        self.assertTrue(result['all_children_reaped']);self.assertEqual(-9,proc.returncode)
        self.assertEqual([(42,15,7),(42,9,9.1)],signals)
        self.assertEqual(9.2,result['cleanup_seconds']) # Includes the injected scheduler delay.

    def test_delay_between_phases_cannot_skip_kill_or_its_grace(self):
        result,_,signals=self.model([0,0,0,7,7,7.1])
        self.assertTrue(result['all_children_reaped'])
        self.assertEqual([(42,15,0),(42,9,7)],signals)

    def test_adopted_children_do_not_restart_an_exhausted_kill_phase(self):
        result,_,signals=self.model([0,0,0,2.1,2.1,5.2,5.3],stuck=True,adopted=True)
        self.assertFalse(result['all_children_reaped']);self.assertEqual([42,43],[p['pid'] for p in result['remaining']])
        self.assertEqual([(42,15,0),(42,9,2.1),(43,9,5.2)],signals)
        self.assertEqual(5.3,result['cleanup_seconds'])


class CommandDeadlineMetadataTests(unittest.TestCase):
    def test_all_platforms_retain_the_actual_bounded_command_deadline(self):
        with tempfile.TemporaryDirectory(prefix='renderer-deadline-metadata-') as name:
            root=Path(name)
            for platform,helper in (('linux','supervise'),('darwin','_posix_command'),('win32','_windows_command')):
                for timeout in (1.25,300):
                    where=root/(platform+str(timeout));where.mkdir()
                    record=dict(evidence_kind='renderer-posix-command',launcher_exit_status=0,
                        launcher_timed_out=False,process_group_retired=True,all_children_retired=True)
                    if platform=='linux':
                        record=good_scope()
                        for key in ('runtime_directory_removed','display_files_retired'):record.pop(key)
                        for file in ('launcher.stdout.log','launcher.stderr.log'):(where/file).write_bytes(b'')
                    with self.subTest(platform=platform,timeout=timeout),patch.object(processes.sys,'platform',platform), \
                            patch.object(processes,helper,return_value=record) as execute:
                        result=processes.run_command([sys.executable],where,timeout)
                    self.assertEqual(timeout,result['timeout_seconds'])
                    self.assertEqual(timeout,json.loads((where/'processes.json').read_text())['timeout_seconds'])
                    self.assertEqual(timeout,execute.call_args.args[2])

    def test_invalid_deadlines_do_not_start_a_command(self):
        with tempfile.TemporaryDirectory(prefix='renderer-invalid-deadline-') as name, \
                patch.object(processes,'_posix_command') as execute:
            for timeout in (0,-1,300.01,float('inf'),float('nan'),True):
                with self.subTest(timeout=timeout),self.assertRaises(ValueError):
                    processes.run_command([sys.executable],Path(name),timeout)
            execute.assert_not_called()


class MacWaitidTests(unittest.TestCase):
    def test_python_api_preserves_nonblocking_nonreaping_flags_and_identity(self):
        with patch.object(processes.sys,'platform','darwin'),patch.object(processes.os,'waitid',create=True) as wait:
            with patch.multiple(processes.os,P_PID=1,WEXITED=4,WNOHANG=1,WNOWAIT=32,create=True):
                wait.return_value=None;self.assertIsNone(processes._mac_waitid(42))
                wait.return_value=SimpleNamespace(si_pid=42);self.assertEqual(42,processes._mac_waitid(42))
                wait.assert_called_with(os.P_PID,42,os.WEXITED|os.WNOHANG|os.WNOWAIT)
                wait.return_value=SimpleNamespace(si_pid=43)
                with self.assertRaisesRegex(ValueError,'foreign child'):processes._mac_waitid(42)

    def test_ctypes_adapter_checks_layout_flags_errors_and_foreign_children(self):
        class Wait:
            observed,error=0,0
            def __call__(self,kind,pid,info,flags):
                self.arguments=(kind,pid,flags);info._obj.pid=self.observed
                ctypes.set_errno(self.error);return -1 if self.error else 0
        wait=Wait()
        self.assertEqual((104,12),(ctypes.sizeof(processes._DarwinSiginfo),processes._DarwinSiginfo.pid.offset))
        with patch.object(processes.sys,'platform','darwin'),patch.object(processes.os,'waitid',None,create=True), \
             patch.multiple(processes.os,P_PID=1,WEXITED=4,WNOHANG=1,WNOWAIT=32,create=True), \
             patch.object(processes.ctypes,'CDLL',return_value=SimpleNamespace(waitid=wait)):
            self.assertIsNone(processes._mac_waitid(42));wait.observed=42
            self.assertEqual(42,processes._mac_waitid(42));self.assertEqual((os.P_PID,42,os.WEXITED|os.WNOHANG|os.WNOWAIT),wait.arguments)
            wait.observed=43
            with self.assertRaisesRegex(ValueError,'foreign child'):processes._mac_waitid(42)
            wait.error=errno.ECHILD
            with self.assertRaises(OSError) as failed:processes._mac_waitid(42)
            self.assertEqual(errno.ECHILD,failed.exception.errno)
            with patch.object(processes.ctypes,'sizeof',return_value=4),self.assertRaisesRegex(ValueError,'siginfo ABI'):
                processes._mac_waitid(42)

    def test_observation_leaves_exit_status_waitable_on_native_mac_when_available(self):
        # Parser/ABI models run everywhere; actual Darwin calls run on that host.
        self.assertEqual(8,ctypes.alignment(processes._DarwinSiginfo))
        if sys.platform!='darwin':return
        for fallback in (False,True):
            proc=subprocess.Popen([sys.executable,'-B','-c','raise SystemExit(7)'],start_new_session=True)
            try:
                with patch.object(processes.os,'waitid',None,create=True) if fallback else nullcontext():
                    deadline=time.monotonic()+3
                    while processes._mac_waitid(proc.pid) is None:
                        self.assertLess(time.monotonic(),deadline);time.sleep(0.01)
                    self.assertEqual(proc.pid,processes._mac_waitid(proc.pid))
                    self.assertIsNone(proc.returncode);self.assertEqual(7,proc.wait(timeout=1))
            finally:
                if proc.returncode is None:proc.kill();proc.wait(timeout=1)


class IsolatedRunnerTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        root = Path(self.temp.name)
        app = root / 'fixture-app'
        app.write_bytes(b'CPU fixture; not a graphics executable')
        self.args = argparse.Namespace(app=app, backend='vulkan', device='hardware', mode='offscreen',
            report_dir=root / 'reports', timeout=1, expected_revision=None, expected_arch='arm64',
            expected_dpr=None, expected_qpa='xcb', background='opaque', scenario='standard',
            icd_manifest=[], weston=None, expect_failure=None, xvfb_run=app)
        self.scope = good_scope()
        self.mutation = None

    def execute(self, command, directory, timeout, env, tools):
        self.assertNotIn('DISPLAY', env)
        self.assertNotIn('XAUTHORITY', env)
        self.assertNotIn('WAYLAND_DISPLAY', env)
        self.assertNotIn('WAYLAND_SOCKET', env)
        self.assertEqual('xcb', env['QT_QPA_PLATFORM'])
        self.assertTrue(command[2].startswith('--auth-file='))
        self.assertTrue(Path(command[2].split('=', 1)[1]).parent.name.startswith('lc-xvfb-'))
        report = directory / 'app.json'
        report.write_text('{}')
        for name in ('stdout.log', 'stderr.log', 'launcher.stdout.log', 'launcher.stderr.log'):
            (directory / name).write_bytes(b'CPU fixture log')
        (directory / 'display.json').write_text('{}')
        evidence = {'schema_version': 3, 'evidence_kind': 'r0-spike-runner', **runner.helper_digests(),
                    'app_sha256': runner.sha256(self.args.app.read_bytes()), 'app_report': {},
                    'app_report_sha256': runner.sha256(report.read_bytes()),
                    'checks': {'executed_smoke_ids': ['fixture:only'], 'failed': 0, 'skipped': 0}}
        (directory / 'candidate.json').write_text(json.dumps(evidence))
        if self.mutation:
            self.mutation(directory, evidence)
        return copy.deepcopy(self.scope)

    def run_isolated(self):
        with patch.object(runner.sys, 'platform', 'linux'), \
             patch.object(runner.shutil, 'which', return_value=str(self.args.app)), \
             patch.object(runner.tempfile, 'TemporaryDirectory',
                          side_effect=lambda **kw: TEMP_DIRECTORY(prefix=kw['prefix'], dir=self.args.report_dir.parent)), \
             patch.object(runner, 'supervise', side_effect=self.execute), \
             patch.object(runner, 'validate_display_retired'), \
             patch.object(runner, 'validate_report', return_value=['fixture:only']):
            return runner.run_isolated(self.args, ['--spike', '--xvfb-run', str(self.args.app)])

    def test_final_report_is_published_only_after_cleanup_and_binds_diagnostics(self):
        directory = self.run_isolated()
        evidence = json.loads((directory / 'checks.json').read_text())
        self.assertFalse((directory / 'candidate.json').exists())
        self.assertTrue(evidence['process_scope']['runtime_directory_removed'])
        for name, digest in evidence['process_artifacts_sha256'].items():
            self.assertEqual(runner.sha256((directory / name).read_bytes()), digest)

    def test_launcher_failure_or_timeout_never_publishes_a_staged_success(self):
        for change in ({'launcher_exit_status': 7}, {'launcher_timed_out': True},
                       {'all_children_reaped': False}, {'errors': ['cleanup failed']}):
            self.scope = {**good_scope(), **change}
            with self.subTest(change=change), self.assertRaisesRegex(ValueError, 'cleanup failed'):
                self.run_isolated()
        self.assertFalse(list(self.args.report_dir.rglob('checks.json')))
        self.assertEqual(4, len(list(self.args.report_dir.rglob('processes.json'))))

    def test_changed_binary_report_or_unreported_late_output_fails(self):
        for mutation in (lambda d, e: self.args.app.write_bytes(b'changed'),
                         lambda d, e: (d / 'app.json').write_text('{"late":true}'),
                         lambda d, e: (d / 'late.png').write_bytes(b'undeclared')):
            self.args.app.write_bytes(b'CPU fixture; not a graphics executable')
            self.mutation = mutation
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                self.run_isolated()
        self.assertFalse(list(self.args.report_dir.rglob('checks.json')))

    def test_wrong_platform_api_or_unmanaged_wayland_never_starts(self):
        for key, value in (('backend', 'metal'), ('expected_qpa', 'offscreen'), ('expected_qpa', 'wayland')):
            saved = getattr(self.args, key)
            setattr(self.args, key, value)
            with patch.object(runner.sys, 'platform', 'linux'), patch.object(runner, 'supervise') as start, self.assertRaises(ValueError):
                runner.run_isolated(self.args, [])
            start.assert_not_called()
            setattr(self.args, key, saved)

    def test_validation_receipt_cannot_be_added_without_requested_profile(self):
        def mutate(directory, evidence):
            evidence['vulkan_validation'] = {'profile': 'unrequested-fixture'}
            (directory / 'candidate.json').write_text(json.dumps(evidence))
        self.mutation = mutate
        with self.assertRaisesRegex(ValueError, 'validation evidence changed'):
            self.run_isolated()
        self.assertFalse(list(self.args.report_dir.rglob('checks.json')))

    def test_validation_rechecks_logs_settings_and_provider_after_worker_exit(self):
        library = self.args.app.parent / 'libVkLayer_khronos_validation.so'
        manifest = self.args.app.parent / 'VkLayer_khronos_validation.json'
        manifest.write_text(json.dumps({'layer': {'name': 'VK_LAYER_KHRONOS_validation',
            'type': 'GLOBAL', 'library_path': library.name, 'api_version': '1.4.341'}}))
        self.args.vulkan_validation_manifest, self.args.vulkan_validation_library = manifest, library
        for changed in ('stdout.log', 'vk_layer_settings.txt', 'library'):
            library.write_bytes(b'CPU-only fake library')
            def mutate(directory, evidence):
                profile = runner.validation_inputs(self.args)
                runner.validation_environment(profile, directory, {})
                (directory / 'stdout.log').write_text('Khronos Validation Layer Active:\n'
                    ' Current Enables: VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT.\n'
                    ' Current Disables: None.\n')
                (directory / 'stderr.log').write_text(('1: calling init: {0}\n'
                    'Insert instance layer "VK_LAYER_KHRONOS_validation" ({0})\n'
                    'Inserted device layer "VK_LAYER_KHRONOS_validation" ({0})\n').format(library))
                evidence['vulkan_validation'] = runner.validate_vulkan_validation(profile, directory)
                (directory / 'candidate.json').write_text(json.dumps(evidence))
                target = library if changed == 'library' else directory / changed
                target.write_bytes(target.read_bytes() + b'changed-after-worker')
            self.mutation = mutate
            with self.subTest(changed=changed), self.assertRaisesRegex(ValueError, 'changed'):
                self.run_isolated()
        self.assertFalse(list(self.args.report_dir.rglob('checks.json')))


@unittest.skipUnless(sys.platform == 'linux', 'Native Linux process adoption/pidfd qualification')
class NativeLinuxProcessTests(unittest.TestCase):
    SUPERVISOR = textwrap.dedent('''
        import json,sys
        from pathlib import Path
        sys.path.insert(0, sys.argv[1])
        import renderer_process as processes
        if len(sys.argv)>5 and float(sys.argv[5]):
            from types import SimpleNamespace
            original_retire,original_time=processes._retire,processes.time
            calls=[0]
            def delayed_clock():
                calls[0]+=1
                return original_time.monotonic()+(float(sys.argv[5]) if calls[0]>=int(sys.argv[6]) else 0)
            def delayed_retire(*args):
                processes.time=SimpleNamespace(monotonic=delayed_clock,sleep=original_time.sleep)
                try:return original_retire(*args)
                finally:processes.time=original_time
            processes._retire=delayed_retire
        record = processes.supervise([sys.executable, '-B', '-c', sys.argv[3]], Path(sys.argv[2]),
                           float(sys.argv[4]), term_grace=0.1, kill_grace=2)
        print(json.dumps(record))
    ''')
    DAEMON = textwrap.dedent('''
        import os,signal,time
        r,w = os.pipe()
        child = os.fork()
        if child == 0:
            os.close(r)
            os.setsid()
            if os.fork(): os._exit(0)
            signal.signal(signal.SIGTERM, signal.SIG_IGN)
            os.write(w,b'ready')
            os.close(w)
            time.sleep(60)
            os._exit(0)
        os.close(w)
        assert os.read(r,5) == b'ready'
        os.close(r)
        os.waitpid(child,0)
    ''')

    def run_fixture(self, payload, timeout=3, cleanup_delay=0, delay_after=2):
        with tempfile.TemporaryDirectory() as name:
            completed = subprocess.run([sys.executable, '-B', '-c', self.SUPERVISOR,
                str(SCRIPTS), name, payload, str(timeout),str(cleanup_delay),str(delay_after)], capture_output=True, timeout=10, check=True)
            record = json.loads(completed.stdout)
        self.assertTrue(record['all_children_reaped'], record)
        self.assertEqual([], record['remaining'])
        for item in record['reaped_at_cleanup']:
            try:
                current = processes.identity(item['pid'])
            except FileNotFoundError:
                continue
            self.assertNotEqual(item['start_ticks'], current['start_ticks'], item)
        return record

    def test_double_fork_setsid_and_stubborn_descendant_are_reaped(self):
        for _ in range(3):
            record = self.run_fixture(self.DAEMON)
            self.assertEqual(0, record['launcher_exit_status'])
            self.assertEqual([], record['errors'])
            self.assertTrue(any(i['signal'] == signal.SIGKILL for i in record['signals']))
            self.assertTrue(any(i['exit_status'] == -signal.SIGKILL for i in record['reaped_at_cleanup']))

    def test_failed_launcher_and_signal_crash_keep_their_original_status(self):
        for payload, code in ((self.DAEMON + '\nraise SystemExit(7)', 7),
                              ('import os,resource,signal; resource.setrlimit(resource.RLIMIT_CORE,(0,0)); os.kill(os.getpid(),signal.SIGABRT)', -signal.SIGABRT)):
            record = self.run_fixture(payload)
            self.assertEqual(code, record['launcher_exit_status'])
            with self.assertRaises(ValueError):
                processes.validate_scope({**record, 'runtime_directory_removed': True})

    def test_timeout_retires_the_launcher_and_escaped_descendant(self):
        record = self.run_fixture(self.DAEMON + '\ntime.sleep(60)', timeout=0.5)
        self.assertTrue(record['launcher_timed_out'])
        self.assertLess(record['launcher_exit_status'], 0)
        self.assertEqual([], record['errors'])

    def test_clock_jumps_before_and_between_signals_still_reap_the_native_tree(self):
        for delay_after in (2,4):
            with self.subTest(delay_after=delay_after):
                record=self.run_fixture(self.DAEMON+'\ntime.sleep(60)',timeout=0.5,cleanup_delay=3,delay_after=delay_after)
                self.assertTrue(record['launcher_timed_out']);self.assertLess(record['launcher_exit_status'],0)
                self.assertEqual([],record['errors']);self.assertGreaterEqual(record['cleanup_seconds'],3)
                self.assertEqual((0.1,2),(record['term_grace_seconds'],record['kill_grace_seconds']))
                self.assertTrue(any(s['signal']==signal.SIGTERM for s in record['signals']))
                self.assertTrue(any(s['signal']==signal.SIGKILL for s in record['signals']))

    def test_preexisting_process_is_untouched_and_childful_parent_is_rejected(self):
        outsider = subprocess.Popen([sys.executable, '-c', 'import time; time.sleep(60)'])
        try:
            self.run_fixture(self.DAEMON)
            self.assertIsNone(outsider.poll())
            with tempfile.TemporaryDirectory() as name, self.assertRaisesRegex(ValueError, 'child-free'):
                processes.supervise([sys.executable, '-c', 'pass'], Path(name), 1)
            self.assertIsNone(outsider.poll())
        finally:
            outsider.terminate()
            outsider.wait(timeout=5)

    def test_ignored_sigchld_is_rejected_without_losing_child_status(self):
        previous = signal.signal(signal.SIGCHLD, signal.SIG_IGN)
        try:
            with tempfile.TemporaryDirectory() as name, self.assertRaisesRegex(ValueError, 'SIGCHLD'):
                processes.supervise([sys.executable, '-c', 'pass'], Path(name), 1)
        finally:
            signal.signal(signal.SIGCHLD, previous)

    def test_multithreaded_caller_is_rejected_before_adopting_unrelated_work(self):
        ready, stop = threading.Event(), threading.Event()
        thread = threading.Thread(target=lambda: (ready.set(), stop.wait(timeout=3)))
        thread.start()
        try:
            self.assertTrue(ready.wait(timeout=1))
            with tempfile.TemporaryDirectory() as name, self.assertRaisesRegex(ValueError, 'single-threaded'):
                processes.supervise([sys.executable, '-c', 'pass'], Path(name), 1)
        finally:
            stop.set()
            thread.join(timeout=2)

    def test_missing_executable_has_no_children_and_preserves_startup_error(self):
        code = self.SUPERVISOR.replace("[sys.executable, '-B', '-c', sys.argv[3]]", "['/nonexistent/librecad-r0-fixture']")
        with tempfile.TemporaryDirectory() as name:
            result = subprocess.run([sys.executable, '-B', '-c', code, str(SCRIPTS), name, '', '1'],
                                    capture_output=True, timeout=10, check=True)
        record = json.loads(result.stdout)
        self.assertTrue(record['all_children_reaped'])
        self.assertIsNone(record['launcher_exit_status'])
        self.assertIn('FileNotFoundError', record['errors'][0])

    def test_supervisor_interrupt_still_reaps_its_owned_tree(self):
        with tempfile.TemporaryDirectory() as name:
            command = [sys.executable, '-B', '-c', self.SUPERVISOR, str(SCRIPTS), name,
                       self.DAEMON + '\nprint("fixture-ready",flush=True)\ntime.sleep(60)', '5']
            proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            try:
                log = Path(name) / 'launcher.stdout.log'
                deadline = time.monotonic() + 3
                while time.monotonic() < deadline and (not log.exists() or 'fixture-ready' not in log.read_text()):
                    self.assertIsNone(proc.poll())
                    time.sleep(0.01)
                self.assertIn('fixture-ready', log.read_text())
                proc.send_signal(signal.SIGTERM) # Only this newly created fixture supervisor.
                out, err = proc.communicate(timeout=5)
                self.assertEqual(0, proc.returncode, err)
                record = json.loads(out)
                self.assertEqual(signal.SIGTERM, record['interrupted_signal'])
                self.assertTrue(record['all_children_reaped'], record)
                self.assertTrue(record['errors'])
            finally:
                if proc.poll() is None:
                    proc.terminate()
                    proc.communicate(timeout=5)

    @unittest.skipUnless(all(shutil.which(name) for name in ('xvfb-run', 'Xvfb', 'xauth')), 'Native Xvfb/xauth provider required')
    def test_real_display_app_failure_and_timeout_publish_no_success(self):
        for body, diagnostic in (('raise SystemExit(7)', 'renderer exited 7'),
                                 (self.DAEMON + '\ntime.sleep(60)', 'renderer timed out')):
            with tempfile.TemporaryDirectory() as name:
                root = Path(name)
                app = root / 'fixture-app'
                app.write_text('#!' + sys.executable + '\n' + body)
                app.chmod(0o700)
                command = [sys.executable, '-B', str(SCRIPTS / 'run_renderer_checks.py'), '--spike',
                           '--xvfb-run', shutil.which('xvfb-run'), '--app', str(app), '--backend', 'vulkan',
                           '--device', 'hardware', '--mode', 'offscreen', '--expected-qpa', 'xcb',
                           '--timeout', '0.5', '--report-dir', str(root / 'reports')]
                result = subprocess.run(command, capture_output=True, timeout=12)
                self.assertNotEqual(0, result.returncode)
                self.assertIn(diagnostic, result.stderr.decode())
                directory = next((root / 'reports').glob('run-*'))
                record = json.loads((directory / 'processes.json').read_text())
                self.assertTrue(record['all_children_reaped'], record)
                self.assertTrue(record['display_files_retired'], record)
                self.assertTrue(record['runtime_directory_removed'], record)
                self.assertEqual(1, record['launcher_exit_status'])
                self.assertFalse((directory / 'checks.json').exists())

    @unittest.skipUnless(all(shutil.which(name) for name in ('xvfb-run', 'Xvfb', 'xauth')), 'Native Xvfb/xauth provider required')
    def test_compositor_start_failure_reaps_escaped_child_and_removes_socket_directory(self):
        with tempfile.TemporaryDirectory() as name:
            root = Path(name)
            prefix = root / 'provider'
            weston = prefix / 'bin/weston'
            weston.parent.mkdir(parents=True)
            weston.write_text('#!' + sys.executable + '\n' + textwrap.dedent('''
                import os,sys,time
                if '--version' in sys.argv:
                    print('weston 14.0.2')
                    raise SystemExit(0)
                print(os.environ['XDG_RUNTIME_DIR'],flush=True)
                r,w=os.pipe()
                if os.fork()==0:
                    os.close(r); os.setsid(); os.write(w,b'1'); os.close(w)
                    time.sleep(60); os._exit(0)
                os.close(w); assert os.read(r,1)==b'1'; os.close(r)
                raise SystemExit(7)
            '''))
            weston.chmod(0o700)
            for relative in ('lib/' + sysconfig.get_config_var('MULTIARCH') + '/libweston-14/x11-backend.so',
                             'lib/' + sysconfig.get_config_var('MULTIARCH') + '/weston/desktop-shell.so',
                             'libexec/weston-desktop-shell', 'libexec/weston-keyboard'):
                path = prefix / relative
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b'CPU startup-failure fixture, never a graphics provider')
            app = root / 'app'
            app.write_text('#!' + sys.executable + '\nraise AssertionError("app must not launch")')
            app.chmod(0o700)
            result = subprocess.run([sys.executable, '-B', str(SCRIPTS / 'run_renderer_checks.py'), '--spike',
                '--xvfb-run', shutil.which('xvfb-run'), '--app', str(app), '--backend', 'vulkan', '--device', 'hardware',
                '--mode', 'offscreen', '--expected-qpa', 'wayland', '--weston', str(weston), '--weston-backend', 'x11',
                '--report-dir', str(root / 'reports')], capture_output=True, timeout=12)
            self.assertNotEqual(0, result.returncode)
            self.assertIn('before its Wayland socket', result.stderr.decode())
            directory = next((root / 'reports').glob('run-*'))
            scope = json.loads((directory / 'processes.json').read_text())
            self.assertTrue(scope['all_children_reaped'], scope)
            self.assertTrue(scope['display_files_retired'], scope)
            self.assertTrue(scope['runtime_directory_removed'], scope)
            self.assertFalse(Path((directory / 'weston.stdout.log').read_text().strip()).exists())
            self.assertFalse((directory / 'checks.json').exists())


if __name__ == '__main__':
    unittest.main()
