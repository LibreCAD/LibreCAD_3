"""CPU parser/ownership fixtures only; synthetic receipts are never native proof."""
import copy
import ctypes
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import MagicMock, patch
import xml.etree.ElementTree as ET

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import renderer_process as processes
import run_renderer_checks as runner
import validate_renderer_inventory as inventory
from scripts.tests.test_renderer_process import good_scope


class BuildContractFixtures(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.selection=inventory.select_case_requirements(inventory.validate_case_manifest(),'R1',
            'cpu-source/MAC-ARM64/cpu/cpu/none/none')
        cls.corpus=inventory.read_json(inventory.REPO_ROOT/cls.selection['fixtures']['contracts']['path'])['datasets']['cpu-build-options']

    def setUp(self):
        tmp=tempfile.TemporaryDirectory(prefix='build-contract-fixture-');self.addCleanup(tmp.cleanup)
        self.root=Path(tmp.name).resolve()
        self.props={}
        for row in self.selection['requirements']:
            self.props[row['test']]=dict(build_case=row['id'],build_arch='arm64',host_arch='arm64',
                build_fixture_sha256=self.selection['fixtures']['contracts']['sha256'],
                build_sources=''.join(inventory.sha256((inventory.REPO_ROOT/p).read_bytes())+'  '+p+'\n' for p in runner.BUILD_SOURCE_PATHS),
                build_observations=json.dumps({a['observed']:a['expected']['value'] for a in self.corpus[row['variant']]['assertions']}))
        self.write_results()

    def write_results(self):
        for n in ('discovery','tests'):(self.root/n).mkdir(exist_ok=True)
        groups=sorted({n.split('.')[0] for n in self.props});lines=[]
        root=ET.Element('testsuites',tests=str(len(self.props)),failures='0',disabled='0',errors='0')
        for group in groups:
            names=sorted(n for n in self.props if n.startswith(group+'.'));lines.append(group+'.')
            suite=ET.SubElement(root,'testsuite',name=group,tests=str(len(names)),failures='0',disabled='0',errors='0',skipped='0')
            for n in names:
                lines.append('  '+n.split('.')[1])
                case=ET.SubElement(suite,'testcase',classname=group,name=n.split('.')[1],status='run',result='completed')
                props=ET.SubElement(case,'properties')
                for key,value in self.props[n].items():ET.SubElement(props,'property',name=key,value=value)
        (self.root/'discovery/stdout.log').write_text('\n'.join(lines)+'\n')
        ET.ElementTree(root).write(self.root/'tests/tests.xml',encoding='utf-8')

    def result(self):return runner.build_contract_results(self.root,self.selection)

    def test_cpu_deadline_rounding_keeps_the_command_cap_and_expiry(self):
        args=SimpleNamespace(cmake_option=[],case_cmake_option=[],cmake=Path(sys.executable),
            trace_format='auto',expected_revision=None,report_dir=self.root,timeout=300,generator=None)
        def command(argv,where,timeout,env,expected_exit=0):
            self.assertEqual(300,timeout)
            raise ValueError('bounded command reached')
        with patch.object(runner,'build_selection',return_value=self.selection), \
                patch.object(runner.subprocess,'check_output',return_value='cmake version 3.27.0\n'), \
                patch.object(runner.time,'monotonic',return_value=212.007),patch.object(runner,'run_command',side_effect=command):
            with self.assertRaisesRegex(ValueError,'bounded command reached'):runner.run_build_contracts(args)
        with patch.object(runner,'build_selection',return_value=self.selection), \
                patch.object(runner.subprocess,'check_output',return_value='cmake version 3.27.0\n'), \
                patch.object(runner.time,'monotonic',side_effect=[212.007,512.008]),patch.object(runner,'run_command') as run:
            with self.assertRaisesRegex(ValueError,'deadline expired'):runner.run_build_contracts(args)
            run.assert_not_called()

    def test_complete_synthetic_results_keep_exact_existing_ids(self):
        result=self.result()
        self.assertEqual(3,len(result['executed_ids']))
        self.assertEqual(['cpu-build-options.empty','cpu-build-options.limit','cpu-build-options.ordinary'],result['required_ids'])

    def test_missing_extra_failed_and_skipped_native_ids_rejected(self):
        original=copy.deepcopy(self.props)
        for mutation in ('missing','extra','failed','skipped'):
            with self.subTest(mutation=mutation):
                self.props=copy.deepcopy(original)
                if mutation=='missing':self.props.pop(next(iter(self.props)))
                if mutation=='extra':self.props['Other.Test']={}
                self.write_results()
                if mutation in ('failed','skipped'):
                    xml=self.root/'tests/tests.xml';tree=ET.parse(xml)
                    ET.SubElement(tree.find('.//testcase'),'failure' if mutation=='failed' else 'skipped')
                    tree.write(xml)
                with self.assertRaises(ValueError):self.result()

    def test_required_provenance_is_exact_and_not_rebound(self):
        p=self.props[next(iter(self.props))];original=dict(p)
        for key,value in [('build_case','foreign'),('build_fixture_sha256','0'*64),('build_arch','x86_64'),
                          ('host_arch','x86_64'),('build_sources',''),('build_sources',original['build_sources'][::-1])]:
            with self.subTest(key=key):
                p.clear();p.update(original);p[key]=value;self.write_results()
                with self.assertRaises(ValueError):self.result()

    def test_missing_extra_wrong_typed_and_duplicate_observations_rejected(self):
        row=next(r for r in self.selection['requirements'] if r['variant']=='ordinary');p=self.props[row['test']]
        raw=p['build_observations'];values=json.loads(raw)
        for data in ({},dict(values,extra=0),dict(values,**{'qt-discovery-count':False}),
                     dict(values,**{'cpu-contract-target-exists':1})):
            with self.subTest(data=data):
                p['build_observations']=json.dumps(data);self.write_results()
                with self.assertRaises(ValueError):self.result()
        for raw in ('{"x":0,"x":0}','{"x":NaN}'):
            p['build_observations']=raw;self.write_results()
            with self.assertRaises(ValueError):self.result()

    def test_inputs_consumed_with_exact_types_and_protected_sdk_options(self):
        for row in self.selection['requirements']:
            actual=runner.build_case_options(row,self.selection,self.corpus)
            for key,typed in self.corpus[row['variant']]['inputs'].items():self.assertIs(typed['value'],actual[runner.BUILD_INPUT_FLAGS[key]])
        for key,typed in [('unknown',dict(type='boolean',unit='none',value=False)),
                          ('rhi',dict(type='boolean',unit='none',value=0)),('rhi',dict(type='integer',unit='none',value=0))]:
            data=copy.deepcopy(self.corpus);data['ordinary']['inputs'][key]=typed
            with self.assertRaises(ValueError):runner.build_case_options(dict(variant='ordinary'),self.selection,data)
        self.assertEqual({'CMAKE_TOOLCHAIN_FILE':'/sdk/toolchain.cmake','LUA_LIBRARY':'/sdk/a;b'},
            runner.build_sdk_options(['CMAKE_TOOLCHAIN_FILE=/sdk/toolchain.cmake','LUA_LIBRARY=/sdk/a;b']))
        for values in (['WITH_RHI_RENDERER=ON'],['LC_CONAN_WITH_QT_UI=ON'],['CMAKE_DISABLE_FIND_PACKAGE_Qt6=OFF'],
             ['CMAKE_RUNTIME_OUTPUT_DIRECTORY=/outside'],['LC_BUILD_CONTRACT_RUN=/foreign'],['CMAKE_INSTALL_PREFIX=/outside'],
             ['x'],['a=1','a=2'],['bad-key=1'],['LUA_LIBRARY=a\nb']):
            with self.subTest(values=values),self.assertRaises(ValueError):runner.build_sdk_options(values)

    def test_cpu_selection_rejects_graphics_cross_arch_and_unimplemented_subsets(self):
        args=SimpleNamespace(configuration=self.selection['configuration_id'],expected_arch=None,scenario='standard',
                             background='opaque',expected_dpr=None,expect_failure=False)
        for k in ('app','build_dir','provider_prefix','xvfb_run','_worker_directory','weston','weston_prefix','weston_input_tool',
                  'icd_manifest','vulkan_validation_manifest','vulkan_validation_library','expected_qpa','backend','device','mode'):
            setattr(args,k,None)
        with patch.object(runner,'native_platform',return_value='MAC-ARM64'):
            self.assertEqual(3,len(runner.build_selection(args)['requirements']))
            for key,value in [('app',self.root/'app'),('backend','metal'),('expected_arch','x86_64')]:
                changed=copy.copy(args);setattr(changed,key,value)
                with self.subTest(key=key),self.assertRaises(ValueError):runner.build_selection(changed)
            selected=copy.deepcopy(self.selection);selected['requirements'][0]['binding']='runner-check'
            with patch.object(inventory,'select_case_requirements',return_value=selected),self.assertRaisesRegex(ValueError,'unimplemented'):
                runner.build_selection(args)
        with patch.object(runner,'native_platform',return_value='LINUX-ARM64'),self.assertRaises(ValueError):runner.build_selection(args)

    def test_gtest_import_adapter_preserves_pre_320_target_without_using_main(self):
        cmake=shutil.which('cmake');self.assertTrue(cmake,'CMake is required for this CPU protocol fixture')
        text=(inventory.REPO_ROOT/'unittest/CMakeLists.txt').read_text()
        adapter=text.split('if(TARGET GTest::gtest)',1)[1].split('target_link_libraries(lcbuildcontracttest',1)[0]
        for index,(target,selected) in enumerate([('GTest::gtest','GTest::gtest'),('GTest::GTest','GTest::GTest')]):
            directory=self.root/('adapter-'+str(index));directory.mkdir()
            (directory/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.15)\nproject(adapter NONE)\n'
                'add_library('+target+' INTERFACE IMPORTED)\nif(TARGET GTest::gtest)'+adapter+
                '\nif(NOT _build_contract_gtest STREQUAL "'+selected+'")\nmessage(FATAL_ERROR "wrong GTest target")\nendif()\n')
            result=subprocess.run([cmake,'-S',str(directory),'-B',str(directory/'build')],capture_output=True,timeout=20)
            self.assertEqual(0,result.returncode,result.stdout+result.stderr)

    def model(self):
        build=self.root/'build';reply=build/'.cmake/api/v1/reply';reply.mkdir(parents=True,exist_ok=True)
        app=build/('Release/lcbuildcontracttest'+('.exe' if sys.platform=='win32' else ''))
        app.parent.mkdir(exist_ok=True);app.write_bytes(b'synthetic-not-executable')
        files={'index-fixture.json':{'reply':{'client-renderer-build':{'codemodel-v2':{'jsonFile':'model.json'}}}},
            'model.json':{'configurations':[dict(name='Debug',targets=[]),dict(name='Release',targets=[dict(name='lcbuildcontracttest',jsonFile='target.json')])]},
            'target.json':{'artifacts':[{'path':str(app.relative_to(build))}]}}
        for name,data in files.items():(reply/name).write_text(json.dumps(data))
        return build,reply,files,app

    def test_file_api_follows_release_artifact_not_a_guessed_path(self):
        build,_,_,app=self.model();self.assertEqual(app,runner.build_test_artifact(build))

    def test_missing_ambiguous_escaping_and_foreign_file_api_artifacts_fail(self):
        for mutation in ('no-index','two-indices','escape','debug-only','two-release','no-target','foreign','two-artifacts'):
            with self.subTest(mutation=mutation):
                build,reply,files,_=self.model()
                if mutation=='no-index':(reply/'index-fixture.json').unlink()
                elif mutation=='two-indices':(reply/'index-second.json').write_text('{}')
                else:
                    if mutation=='escape':files['index-fixture.json']['reply']['client-renderer-build']['codemodel-v2']['jsonFile']='../outside.json'
                    if mutation=='debug-only':files['model.json']['configurations'].pop()
                    if mutation=='two-release':files['model.json']['configurations'].append(files['model.json']['configurations'][-1])
                    if mutation=='no-target':files['model.json']['configurations'][-1]['targets']=[]
                    if mutation=='foreign':files['target.json']['artifacts'][0]['path']=str(self.root/'outside'/Path(files['target.json']['artifacts'][0]['path']).name)
                    if mutation=='two-artifacts':files['target.json']['artifacts']*=2
                    for name,data in files.items():(reply/name).write_text(json.dumps(data))
                with self.assertRaises(ValueError):runner.build_test_artifact(build)
                for f in reply.glob('*.json'):f.unlink()

    def rejection(self):
        name='RenderBuildContractTest.CpuBuildOptionsOrdinary'
        raw=('<?xml version="1.0"?><testsuites tests="1" failures="1" disabled="0" errors="0">'
             '<testsuite tests="1" failures="1" disabled="0" errors="0" skipped="0">'
             '<testcase classname="RenderBuildContractTest" name="CpuBuildOptionsOrdinary" status="run" result="completed">'
             '<failure><![CDATA[native assertion: qt-discovery-count < 1]]></failure></testcase></testsuite></testsuites>')
        return name,raw

    def test_real_gtest_failure_cdata_is_accepted_not_misread_as_dtd(self):
        name,raw=self.rejection();xml=self.root/'rejection.xml';xml.write_text(raw)
        runner.expected_build_rejection(xml,name,'qt-discovery-count')

    def test_wrong_failure_counts_ids_status_and_diagnostic_cannot_pass(self):
        name,raw=self.rejection();xml=self.root/'rejection.xml'
        changes=[raw.replace('failures="1"','failures="0"',1),raw.replace('skipped="0"','skipped="1"'),
            raw.replace('CpuBuildOptionsOrdinary','Wrong'),raw.replace('status="run"','status="notrun"'),
            raw.replace('qt-discovery-count','other'),raw.replace('<failure>','<skipped>').replace('</failure>','</skipped>'),
            raw.replace('<testsuites ','<!DOCTYPE testsuites><testsuites ',1),
            raw.replace('<testsuites ','<!ENTITY forbidden "x"><testsuites ',1)]
        for data in changes:
            xml.write_text(data)
            with self.subTest(data=data),self.assertRaises(ValueError):runner.expected_build_rejection(xml,name,'qt-discovery-count')


class CommandOwnershipFixtures(unittest.TestCase):
    def test_non_display_scope_and_expected_rejection_never_claim_display_cleanup(self):
        record=good_scope();record.pop('runtime_directory_removed');record.pop('display_files_retired')
        record.update(display_required=False,launcher_exit_status=1)
        processes.validate_scope(record,False,1)
        for changes in ({'launcher_exit_status':0},{'launcher_exit_status':True},{'display_required':True},
                        {'runtime_directory_removed':True},{'display_files_retired':True}):
            with self.subTest(changes=changes),self.assertRaises(ValueError):processes.validate_scope(dict(record,**changes),False,1)
        for display,exit_code in ((0,1),(False,True),(False,2)):
            with self.assertRaises(ValueError):processes.validate_scope(record,display,exit_code)

    def test_mac_libproc_named_wrapper_returns_pid_count_not_bytes(self):
        lib=SimpleNamespace(proc_listpgrppids=MagicMock())
        def query(pid,buffer,size):
            self.assertEqual(12,pid)
            if buffer is None:return 3
            self.assertGreaterEqual(size,3*ctypes.sizeof(ctypes.c_int))
            for i,p in enumerate((12,13,14)):buffer[i]=p
            return 3
        lib.proc_listpgrppids.side_effect=query
        with patch.object(processes.ctypes,'CDLL',return_value=lib):self.assertEqual([12,13,14],processes._mac_group(12))
        lib.proc_listpgrppids.side_effect=[3,67]
        with patch.object(processes.ctypes,'CDLL',return_value=lib),self.assertRaises(OSError):processes._mac_group(12)

    def test_mac_exited_root_only_is_reaped_without_signaling_zombie_group(self):
        order=[]
        with tempfile.TemporaryDirectory() as tmp,patch.object(processes.subprocess,'Popen') as popen, \
             patch.object(processes.os,'pipe',return_value=(101,102)),patch.object(processes.os,'close') as close, \
             patch.object(processes.os,'write',side_effect=lambda *a:order.append('release') or 1) as release, \
             patch.object(processes.os,'getpgid',side_effect=lambda p:order.append('group') or 12,create=True), \
             patch.object(processes.os,'getsid',side_effect=lambda p:order.append('session') or 12,create=True), \
             patch.object(processes,'_mac_waitid',return_value=12), \
             patch.object(processes,'_mac_group',side_effect=[[12],[]]),patch.object(processes.os,'killpg',create=True) as kill:
            popen.return_value.pid=12;popen.return_value.returncode=0
            result=processes._posix_command(['synthetic'],Path(tmp),1,{})
            self.assertTrue(result['process_group_retired']);kill.assert_not_called();popen.return_value.wait.assert_called_once()
            release.assert_called_once_with(102,b'1');self.assertEqual([((101,),),((102,),)],close.call_args_list)
            argv=popen.call_args.args[0];self.assertEqual([sys.executable,'-I','-S','-B','-c'],argv[:5])
            self.assertEqual('101',argv[6]);self.assertEqual('synthetic',argv[7])
            self.assertEqual((101,),popen.call_args.kwargs['pass_fds'])
            self.assertEqual(['group','session','release'],order)

    def test_mac_unverified_session_never_releases_or_signals_a_group(self):
        with tempfile.TemporaryDirectory() as tmp,patch.object(processes.subprocess,'Popen') as popen, \
             patch.object(processes.os,'pipe',return_value=(101,102)),patch.object(processes.os,'close') as close, \
             patch.object(processes.os,'write') as release,patch.object(processes.os,'getpgid',return_value=11,create=True), \
             patch.object(processes.os,'killpg',create=True) as kill:
            popen.return_value.pid=12
            with self.assertRaisesRegex(ValueError,'ownership was not established'):
                processes._posix_command(['synthetic'],Path(tmp),1,{})
            release.assert_not_called();kill.assert_not_called();popen.return_value.terminate.assert_called_once()
            popen.return_value.wait.assert_called_once();self.assertEqual([((101,),),((102,),)],close.call_args_list)

    def test_mac_failed_spawn_closes_both_handshake_descriptors(self):
        with tempfile.TemporaryDirectory() as tmp,patch.object(processes.subprocess,'Popen',side_effect=OSError('spawn failed')), \
             patch.object(processes.os,'pipe',return_value=(101,102)),patch.object(processes.os,'close') as close:
            with self.assertRaisesRegex(OSError,'spawn failed'):processes._posix_command(['synthetic'],Path(tmp),1,{})
            self.assertEqual([((102,),),((101,),)],close.call_args_list)

    def test_helper_exception_retains_explicit_unknown_cleanup_not_success(self):
        with tempfile.TemporaryDirectory() as tmp,patch.object(processes.sys,'platform','darwin'), \
             patch.object(processes,'_posix_command',side_effect=PermissionError('fixture denied')):
            root=Path(tmp)
            with self.assertRaises(PermissionError):processes.run_command([sys.executable],root,1)
            record=json.loads((root/'processes.json').read_text())
            self.assertIs(record['cleanup_verified'],False);self.assertIn('PermissionError',record['error'])

    def windows(self,assign=True,status=0):
        calls=[]
        def function(name):
            def call(*args):
                calls.append(name)
                if name=='CreateJobObjectW':return 10
                if name=='CreateProcessW':
                    self.assertEqual(0x404,args[5]);args[-1]._obj.process=11;args[-1]._obj.thread=12;return 1
                if name=='AssignProcessToJobObject':return int(assign)
                if name=='WaitForSingleObject':return status
                if name=='GetExitCodeProcess':args[1]._obj.value=0
                if name=='QueryInformationJobObject':args[2]._obj.active=0
                return 1
            return call
        names=('CreateJobObjectW','SetInformationJobObject','CreateProcessW','AssignProcessToJobObject','ResumeThread',
               'WaitForSingleObject','GetExitCodeProcess','TerminateJobObject','TerminateProcess','QueryInformationJobObject','CloseHandle')
        return SimpleNamespace(**{name:function(name) for name in names}),calls

    def test_windows_protocol_assigns_suspended_child_before_resume_then_retires_job(self):
        kernel,calls=self.windows()
        with tempfile.TemporaryDirectory() as tmp,patch.object(ctypes,'WinDLL',return_value=kernel,create=True), \
             patch.dict(sys.modules,msvcrt=SimpleNamespace(get_osfhandle=lambda fd:fd)):
            result=processes._windows_command(['synthetic'],Path(tmp),1,{})
        self.assertLess(calls.index('AssignProcessToJobObject'),calls.index('ResumeThread'))
        self.assertLess(calls.index('CreateProcessW'),calls.index('AssignProcessToJobObject'))
        self.assertLess(calls.index('TerminateJobObject'),calls.index('QueryInformationJobObject'))
        self.assertTrue(result['all_children_retired']);self.assertFalse(result['launcher_timed_out'])

    def test_windows_assignment_failure_never_resumes_unowned_child(self):
        kernel,calls=self.windows(assign=False)
        with tempfile.TemporaryDirectory() as tmp,patch.object(ctypes,'WinDLL',return_value=kernel,create=True), \
             patch.object(ctypes,'WinError',return_value=OSError('fixture assign failure'),create=True), \
             patch.object(ctypes,'get_last_error',return_value=5,create=True), \
             patch.dict(sys.modules,msvcrt=SimpleNamespace(get_osfhandle=lambda fd:fd)),self.assertRaises(OSError):
            processes._windows_command(['synthetic'],Path(tmp),1,{})
        self.assertNotIn('ResumeThread',calls);self.assertIn('TerminateProcess',calls)
        self.assertLess(calls.index('TerminateProcess'),calls.index('WaitForSingleObject'))

    def test_windows_timeout_retires_job_but_never_becomes_command_success(self):
        kernel,calls=self.windows(status=258)
        with tempfile.TemporaryDirectory() as tmp,patch.object(ctypes,'WinDLL',return_value=kernel,create=True), \
             patch.dict(sys.modules,msvcrt=SimpleNamespace(get_osfhandle=lambda fd:fd)):
            result=processes._windows_command(['synthetic'],Path(tmp),0.1,{})
            self.assertTrue(result['launcher_timed_out']);self.assertTrue(result['all_children_retired'])
            with patch.object(processes.sys,'platform','win32'),patch.object(processes,'_windows_command',return_value=result),self.assertRaises(ValueError):
                processes.run_command([sys.executable],Path(tmp),0.1)
        self.assertIn('TerminateJobObject',calls)


if __name__=='__main__':unittest.main()
