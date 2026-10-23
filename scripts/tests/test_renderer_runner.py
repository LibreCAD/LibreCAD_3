"""Synthetic CPU failure fixtures for R1 parsing/binding; never native GPU evidence."""
import argparse
import copy
import json
import os
import platform
import shutil
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET
from unittest.mock import patch
from types import SimpleNamespace

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import run_renderer_checks as runner
import validate_renderer_inventory as inventory


class RhiRunnerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.inventory = inventory.validate_case_manifest()
        cls.selection = inventory.select_case_requirements(cls.inventory,'R1',
            'gui-less-macos/MAC-ARM64/rhi-offscreen/offscreen/cocoa/hardware')

    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix='rhi-runner-fixture-')
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name).resolve()
        self.prefix = self.root/'provider'; self.prefix.mkdir()
        self.build = self.root/'build'
        self.shaders = self.build/'lcrenderer/rhi/.qsb'; self.shaders.mkdir(parents=True)
        for n in ('vert','frag'): (self.shaders/('triangles.'+n+'.qsb')).write_bytes(n.encode())
        self.qt = {}
        for n in ('Qt6Core.so','Qt6Gui.so'):
            p=self.prefix/n; p.write_bytes(b'synthetic-module')
            self.qt[str(p)] = inventory.sha256(p.read_bytes())
        self.qsb=self.prefix/'qsb';self.qsb.write_bytes(b'synthetic-qsb')
        headers={}
        for n in ('qrhi.h','qrhi_platform.h','qshaderdescription.h','qshaderdescription_p.h'):
            p=self.prefix/n;p.write_bytes(b'synthetic-private-header');headers[str(p)]=inventory.sha256(p.read_bytes())
        paths=['lcrenderer/rhi/renderer.cpp','lcrenderer/rhi/tests/buildtest.cpp',
               'lcrenderer/rhi/tests/provenance.h','lcviewernoqt/painters/renderframe.cpp']
        inputs=''.join(inventory.sha256((inventory.REPO_ROOT/p).read_bytes())+'  '+p+'\n' for p in sorted(paths))
        self.metadata=dict(schema_version=1,evidence_kind='rhi-test-build-inputs',rhi_inputs=inputs,
            rhi_inputs_sha256=inventory.sha256(inputs.encode()),compiler='synthetic-compiler',qt='6.12.0',
            revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=inventory.REPO_ROOT,text=True).strip(),
            git_dirty=bool(subprocess.check_output(['git','status','--porcelain','--untracked-files=normal'],cwd=inventory.REPO_ROOT)),
            qsb=str(self.qsb),qsb_sha256=inventory.sha256(self.qsb.read_bytes()),
            private_headers=headers,
            options=self.selection['configuration']['expected_options'])
        self.args=argparse.Namespace(provider_prefix=self.prefix,build_dir=self.build,expected_revision=None)
        self.props={}
        for row in self.selection['requirements']:
            body=self.inventory['datasets'][row['fixture']][row['dataset']][row['variant']]
            observed={a['observed']:('metal' if a['expected']['type']=='configuration' else a['expected']['value'])
                      for a in body['assertions']}
            self.props[row['test']]=dict(rhi_case=row['id'],rhi_api='metal',rhi_device_class='hardware',
                rhi_qpa='cocoa',rhi_arch='arm64',rhi_build_arch='arm64',rhi_device='synthetic device',rhi_qt='6.12.0',
                rhi_build=json.dumps(self.metadata),rhi_qt_modules=json.dumps(self.qt),
                rhi_fixture_sha256=self.selection['fixtures']['contracts']['sha256'],
                rhi_shader_sha256=inventory.sha256(b'vertfrag'),rhi_indexed_draw_count='1',
                rhi_texture_limit='4',rhi_texture_size_max='16384',rhi_observations=json.dumps(observed))
        self.props['RenderShaderTest.SyntheticRegression']={}
        self.write_results()

    def write_results(self):
        tests=sorted(self.props)
        suites={n.split('.')[0] for n in tests}
        lines=[]
        root=ET.Element('testsuites',tests=str(len(tests)),failures='0',disabled='0',errors='0')
        for suite in sorted(suites):
            names=[n for n in tests if n.startswith(suite+'.')]
            lines.append(suite+'.')
            group=ET.SubElement(root,'testsuite',name=suite,tests=str(len(names)),failures='0',disabled='0',skipped='0',errors='0')
            for full in names:
                name=full.split('.')[1];lines.append('  '+name)
                test=ET.SubElement(group,'testcase',name=name,classname=suite,status='run',result='completed')
                properties=ET.SubElement(test,'properties')
                for k,v in self.props[full].items(): ET.SubElement(properties,'property',name=k,value=v)
        (self.root/'discovery.txt').write_text('\n'.join(lines)+'\n')
        ET.ElementTree(root).write(self.root/'tests.xml',encoding='utf-8')

    def result(self): return runner.rhi_results(self.args,self.root,self.selection)

    def change_xml(self, change):
        path=self.root/'tests.xml'; tree=ET.parse(path); change(tree.getroot()); tree.write(path,encoding='utf-8')

    def test_complete_synthetic_binding_keeps_regressions_and_diagnostics(self):
        result=self.result()
        self.assertEqual(3,len(result['required_ids']))
        self.assertEqual(4,len(result['executed_ids']))
        self.assertEqual({'synthetic device'},set(result['observed_devices'].values()))
        self.assertEqual('texture-size-limit',result['observations']['rhi-build-options.limit']['diagnostic'])

    def test_missing_required_case_even_when_discovery_and_xml_agree(self):
        del self.props['RenderBuildTest.RhiEmpty'];self.write_results()
        with self.assertRaisesRegex(ValueError,'Required manifest'):self.result()

    def test_discovery_rejects_empty_disabled_duplicate_wildcard_and_noise(self):
        for data in (b'',b'S.\n  DISABLED_A\n',b'S.\n  A\n  A\n',b'S.\n  *\n',b'noise\n',b'S.\n  A\xff\n'):
            with self.subTest(data=data),self.assertRaises((ValueError,UnicodeError)):runner.gtest_discovery(data)

    def test_discovery_parameter_comments_do_not_drop_ids(self):
        self.assertEqual(['S/0.Test/1'],runner.gtest_discovery(b'S/0. # type\n  Test/1 # parameter\n'))

    def test_xml_failure_skip_disabled_notrun_and_id_mismatch(self):
        mutations=[lambda r:r.set('failures','1'),lambda r:r.set('disabled','1'),
                   lambda r:r.find('testsuite').set('skipped','1'),
                   lambda r:r.find('.//testcase').set('result','skipped'),
                   lambda r:r.find('.//testcase').set('status','notrun'),
                   lambda r:r.find('.//testcase').set('name','WrongId'),
                   lambda r:ET.SubElement(r.find('.//testcase'),'failure')]
        for change in mutations:
            with self.subTest(change=change):
                self.write_results();self.change_xml(change)
                with self.assertRaises(ValueError):self.result()

    def test_duplicate_property_and_dtd_xml_rejected(self):
        self.change_xml(lambda r:ET.SubElement(r.find('.//properties'),'property',name='rhi_api',value='metal'))
        with self.assertRaisesRegex(ValueError,'properties'):self.result()
        (self.root/'tests.xml').write_bytes(b'<!DOCTYPE testsuites><testsuites/>')
        with self.assertRaisesRegex(ValueError,'DTD'):self.result()

    def test_native_identity_fields_are_all_required_and_exact(self):
        p=self.props['RenderBuildTest.RhiOrdinary'];original=dict(p)
        for key in ('rhi_api','rhi_device_class','rhi_qpa','rhi_arch','rhi_build_arch','rhi_qt',
                    'rhi_fixture_sha256','rhi_shader_sha256','rhi_case','rhi_device'):
            with self.subTest(key=key):
                p.clear();p.update(original);p[key]='';self.write_results()
                with self.assertRaises(ValueError):self.result()

    def test_each_observation_must_match_with_exact_types_and_full_pixels(self):
        for name,field,value in [('RhiOrdinary','draw-count',True),('RhiOrdinary','actual-backend','vulkan'),
             ('RhiOrdinary','normalized-rgba',[255,0,0,255]),('RhiEmpty','target-ready',0),
             ('RhiLimit','diagnostic','anything'),('RhiLimit','accepted-output-count',1)]:
            p=self.props['RenderBuildTest.'+name]; old=p['rhi_observations'];data=json.loads(old);data[field]=value
            with self.subTest(field=field):
                p['rhi_observations']=json.dumps(data);self.write_results()
                with self.assertRaisesRegex(ValueError,'observation'):self.result()
            p['rhi_observations']=old

    def test_missing_and_extra_observations_rejected(self):
        p=self.props['RenderBuildTest.RhiEmpty']
        for data in ({'draw-count':0},{'draw-count':0,'target-ready':False,'extra':0}):
            p['rhi_observations']=json.dumps(data);self.write_results()
            with self.assertRaisesRegex(ValueError,'observations'):self.result()

    def test_nonindexed_quad_or_native_exhaustion_cannot_pass(self):
        for name,key,value in [('RhiOrdinary','rhi_indexed_draw_count','0'),
            ('RhiLimit','rhi_texture_limit','5'),('RhiLimit','rhi_texture_size_max','4')]:
            p=self.props['RenderBuildTest.'+name];old=p[key];p[key]=value;self.write_results()
            with self.subTest(key=key),self.assertRaises(ValueError):self.result()
            p[key]=old

    def test_build_revision_options_and_input_manifest_rejected_on_change(self):
        for key,value in [('revision','0'*40),('options',dict(self.metadata['options'],WITH_RHI_RENDERER=1)),
                          ('git_dirty',None),('rhi_inputs_sha256','0'*64),('rhi_inputs',''),('qsb_sha256','0'*64),('qt','6.7.0')]:
            data=dict(self.metadata);data[key]=value
            for p in self.props.values():
                if 'rhi_build' in p:p['rhi_build']=json.dumps(data)
            self.write_results()
            with self.subTest(key=key),self.assertRaises(ValueError):self.result()

    def test_source_digest_cannot_be_rebound_to_current_checkout(self):
        data=dict(self.metadata);inputs=data['rhi_inputs'];data['rhi_inputs']='0'*64+inputs[64:]
        data['rhi_inputs_sha256']=inventory.sha256(data['rhi_inputs'].encode())
        for p in self.props.values():
            if 'rhi_build' in p:p['rhi_build']=json.dumps(data)
        self.write_results()
        with self.assertRaisesRegex(ValueError,'source differs'):self.result()

    def test_missing_foreign_and_changed_actual_provider_images(self):
        original=self.props['RenderBuildTest.RhiOrdinary']['rhi_qt_modules']
        for data in ({},dict(self.qt,**{str(self.root/'outside-Qt6Gui.so'):'0'*64}),
                     {p:'0'*64 for p in self.qt}):
            for p in self.props.values():
                if 'rhi_qt_modules' in p:p['rhi_qt_modules']=json.dumps(data)
            self.write_results()
            with self.subTest(data=data),self.assertRaises((ValueError,OSError)):self.result()
        self.assertTrue(original)

    def test_shader_and_qsb_bytes_are_bound(self):
        (self.shaders/'triangles.vert.qsb').write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError,'identity mismatch'):self.result()
        (self.shaders/'triangles.vert.qsb').write_bytes(b'vert');self.qsb.write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError,'shader baker'):self.result()

    def test_private_header_bytes_are_bound(self):
        (self.prefix/'qrhi.h').write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError,'private Qt header'):self.result()

    def test_unselected_case_cannot_claim_canonical_observations(self):
        self.props['RenderShaderTest.SyntheticRegression']={'rhi_case':'rhi-build-options.empty'}
        self.write_results()
        with self.assertRaisesRegex(ValueError,'Foreign/misbound'):self.result()

    def test_duplicate_json_keys_and_nan_are_rejected(self):
        p=self.props['RenderBuildTest.RhiOrdinary']
        for raw in ('{"draw-count":1,"draw-count":1}','{"draw-count":NaN}'):
            p['rhi_observations']=raw;self.write_results()
            with self.assertRaises(ValueError):self.result()

    def test_cli_requires_explicit_nonoverlapping_scope(self):
        for flags in ([],['--spike','--rhi-contracts']):
            p=subprocess.run([sys.executable,'-B',str(Path(runner.__file__)),*flags],capture_output=True,timeout=10)
            self.assertEqual(2,p.returncode)

    def run_args(self):
        app=self.root/'synthetic-app';app.write_bytes(b'never executed; mocked child protocol')
        return argparse.Namespace(**vars(self.args),configuration=self.selection['configuration_id'],
            app=app,report_dir=self.root/'reports',_worker_directory=None,weston=None,weston_prefix=None,
            weston_backend='headless',backend='metal',device='hardware',mode='offscreen',expected_qpa='cocoa',
            expected_arch=None,expected_dpr=None,expect_failure=None,scenario='standard',background='opaque',
            icd_manifest=[],timeout=10,vulkan_validation_manifest=None,vulkan_validation_library=None)

    def simulate(self, failure=None):
        args=self.run_args()
        discovery=(self.root/'discovery.txt').read_bytes();xml=(self.root/'tests.xml').read_bytes()
        real_run=subprocess.run
        calls=[]
        def child(command,**kwargs):
            if command[0]=='git':return real_run(command,**kwargs)
            calls.append(command)
            env=kwargs['env']
            self.assertFalse(any(k.startswith('GTEST_') for k in env))
            self.assertEqual(('metal','hardware','cocoa'),tuple(env[k] for k in
                ('LC_RENDERER_EXPECT_API','LC_RENDERER_EXPECT_DEVICE','QT_QPA_PLATFORM')))
            if failure=='timeout':raise subprocess.TimeoutExpired(command,10,output=b'partial output')
            if failure=='exit':return subprocess.CompletedProcess(command,2,b'failed',b'diagnostic')
            if '--gtest_list_tests' in command:return subprocess.CompletedProcess(command,0,discovery,b'')
            if failure!='xml':Path(command[1].split('xml:',1)[1]).write_bytes(xml)
            return subprocess.CompletedProcess(command,0,b'fixture complete',b'')
        with patch.object(runner.sys,'platform','darwin'),patch.object(runner,'rhi_selection',return_value=self.selection),\
             patch.dict(os.environ,{'GTEST_FILTER':'NoMatch','LC_RENDERER_EXPECT_API':'wrong'}),\
             patch.object(runner.subprocess,'run',side_effect=child):
            if failure:
                with self.assertRaises((ValueError,OSError)):runner.run_rhi(args)
                self.assertEqual([],list(args.report_dir.rglob('checks.json')))
            else:
                directory=runner.run_rhi(args)
                evidence=json.loads((directory/'checks.json').read_text())
                self.assertIs(False,evidence['phase_or_platform_accepted'])
                self.assertEqual('r1-rhi-contract-runner',evidence['evidence_kind'])
                self.assertEqual(4,len(evidence['rhi']['executed_ids']))
                self.assertEqual(2,len(calls))

    def test_driver_executes_discovery_and_all_ids_despite_environment_filter(self):self.simulate()
    def test_driver_nonzero_exit_retains_failure_without_success(self):self.simulate('exit')
    def test_driver_timeout_retains_failure_without_success(self):self.simulate('timeout')
    def test_driver_missing_xml_cannot_pass(self):self.simulate('xml')

    def test_native_tuple_scope_options_cannot_be_ignored(self):
        args=self.run_args()
        with patch.object(runner.sys,'platform','linux'),patch.object(runner.platform,'machine',return_value='aarch64'):
            args.configuration='gui-less-linux/LINUX-ARM64/rhi-offscreen/offscreen/xcb/software'
            args.backend='vulkan';args.device='software';args.expected_qpa='xcb'
            self.assertEqual(3,len(runner.rhi_selection(args)['requirements']))
            for key,value in [('device','any'),('mode','widget'),('backend','metal'),('expected_qpa','wayland'),
                              ('expected_arch','x86_64'),('build_dir',None),('provider_prefix',None),('scenario','recovery')]:
                changed=copy.copy(args);setattr(changed,key,value)
                with self.subTest(key=key),self.assertRaises(ValueError):runner.rhi_selection(changed)

    def test_translated_mac_cannot_claim_native_tuple(self):
        import ctypes
        args=self.run_args()
        def translated(name,value,size,old,length):value._obj.value=1;return 0
        with patch.object(runner.sys,'platform','darwin'),patch.object(runner.platform,'machine',return_value='arm64'),\
             patch.object(ctypes,'CDLL',return_value=SimpleNamespace(sysctlbyname=translated)):
            with self.assertRaisesRegex(ValueError,'translated'):runner.rhi_selection(args)

    def test_supervised_finalizer_rejects_failed_cleanup_before_reading_candidate(self):
        with self.assertRaisesRegex(ValueError,'cleanup'):
            runner.finish_rhi(self.args,self.root,None,runner.helper_digests(),{'launcher_exit_status':1})

    def test_workflow_executes_new_failure_fixtures(self):
        workflow=(inventory.REPO_ROOT/'.github/workflows/renderer.yml').read_text()
        contracts=workflow.split('  renderer-contracts:',1)[1].split('  renderer-linux:',1)[0]
        self.assertIn('scripts.tests.test_renderer_runner',contracts)

    def test_private_header_lookup_uses_only_reviewed_provider_paths(self):
        cmake=(inventory.REPO_ROOT/'lcrenderer/rhi/CMakeLists.txt').read_text()
        block='    get_target_property(_private_includes'+cmake.split('    get_target_property(_private_includes',1)[1].split('    set(RHI_PRIVATE_HEADERS_JSON',1)[0]
        sdk=self.root/'declared-headers';sdk.mkdir()
        for name in ('rhi/qrhi.h','rhi/qrhi_platform.h','rhi/qshaderdescription.h','private/qshaderdescription_p.h'):
            p=sdk/name;p.parent.mkdir(exist_ok=True);p.write_text('// synthetic\n')
        guard='$<$<BOOL:$<TARGET_PROPERTY:Qt6::Gui,_qt_module_has_private_headers>>:'+str(sdk)+'>'
        for i,(path,flag,success) in enumerate(((str(sdk),'TRUE',True),(guard,'TRUE',True),
                (guard,'FALSE',False),('$<BUILD_INTERFACE:'+str(sdk)+'>','TRUE',False),
                (str(self.root/'missing-headers'),'TRUE',False))):
            source=self.root/('probe'+str(i));source.mkdir()
            (source/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.21)\nproject(PrivatePaths NONE)\n'
                'add_library(Qt6::Gui INTERFACE IMPORTED)\nadd_library(Qt6::GuiPrivate INTERFACE IMPORTED)\n'
                'set_property(TARGET Qt6::Gui PROPERTY _qt_module_has_private_headers '+flag+')\n'
                'set_property(TARGET Qt6::GuiPrivate PROPERTY INTERFACE_INCLUDE_DIRECTORIES "'+path+'")\n'+block+
                'list(REMOVE_DUPLICATES _private_headers)\nlist(LENGTH _private_headers count)\n'
                'if(NOT count EQUAL 4)\nmessage(FATAL_ERROR "Four distinct headers required")\nendif()\n')
            p=subprocess.run(['cmake','-S',str(source),'-B',str(source/'build')],capture_output=True,text=True,timeout=20)
            with self.subTest(style=i):self.assertEqual(success,p.returncode==0,p.stdout+p.stderr)

    def test_software_icd_selection_is_exact_and_content_bound(self):
        first=self.root/'first.json';second=self.root/'second.json'
        first.write_text('{}');second.write_text('{}')
        args=argparse.Namespace(device='software',icd_manifest=[first])
        with patch.dict(os.environ,{'VK_DRIVER_FILES':str(first)}):
            self.assertEqual(inventory.sha256(first.read_bytes()),runner.rhi_icds(args)[0]['sha256'])
            before=runner.rhi_icds(args);first.write_text('{"changed":true}')
            self.assertNotEqual(before,runner.rhi_icds(args))
        with patch.dict(os.environ,{'VK_DRIVER_FILES':str(first)+os.pathsep+str(second)}):
            with self.assertRaisesRegex(ValueError,'exactly declared'):runner.rhi_icds(args)

    def test_supervised_publish_rechecks_artifacts_scope_and_observations(self):
        args=self.run_args();helpers=runner.helper_digests()
        record=dict(schema_version=1,evidence_kind='r0-process-scope',remaining=[],errors=[],launcher_exit_status=0,
            launcher_timed_out=False,interrupted_signal=None,root_pid=12345,cleanup_seconds=0.01,reaped_at_cleanup=[],signals=[])
        record.update({n:True for n in ('fresh_child_free_parent','subreaper','subreaper_restored','pidfd_signals',
            'all_children_reaped','runtime_directory_removed','display_files_retired')})
        for mutation in (None,'log','extra','scope','dropped-log','observation','helper','host','command'):
            directory=self.root/('finalize-'+str(mutation));directory.mkdir()
            for n in ('discovery.txt','tests.xml'):shutil.copy2(self.root/n,directory/n)
            for n in ('discovery.stderr.log','stdout.log','stderr.log','display.json','processes.json',
                      'launcher.stdout.log','launcher.stderr.log'):(directory/n).write_text('synthetic artifact\n')
            artifacts={n:inventory.sha256((directory/n).read_bytes()) for n in
                ('discovery.txt','tests.xml','discovery.stderr.log','stdout.log','stderr.log','display.json')}
            evidence=dict(schema_version=1,evidence_kind='r1-rhi-contract-runner',phase_or_platform_accepted=False,
                app_sha256=inventory.sha256(args.app.read_bytes()),**helpers,rhi=runner.rhi_results(args,directory,self.selection),
                software_icds=[],vulkan_validation=None,artifacts_sha256=artifacts,
                host=dict(system=platform.system(),release=platform.release(),machine=platform.machine()),
                commands=[[str(args.app),'--gtest_list_tests'],[str(args.app),'--gtest_output=xml:'+str(directory/'tests.xml')]])
            if mutation=='log':(directory/'stdout.log').write_text('changed')
            if mutation=='extra':(directory/'extra.txt').write_text('unexpected')
            if mutation=='scope':evidence['phase_or_platform_accepted']=True
            if mutation=='dropped-log':(directory/'stdout.log').unlink();del artifacts['stdout.log']
            if mutation=='observation':evidence['rhi']['observations']['rhi-build-options.empty']['draw-count']=1
            if mutation=='helper':evidence['runner_sha256']='0'*64
            if mutation=='host':evidence['host']['machine']='foreign'
            if mutation=='command':evidence['commands'][0].append('--gtest_filter=NoMatch')
            (directory/'candidate.json').write_text(json.dumps(evidence))
            with self.subTest(mutation=mutation),patch.object(runner,'rhi_selection',return_value=self.selection):
                if mutation:
                    with self.assertRaises(ValueError):runner.finish_rhi(args,directory,None,helpers,record)
                    self.assertFalse((directory/'checks.json').exists())
                else:
                    runner.finish_rhi(args,directory,None,helpers,record)
                    self.assertTrue((directory/'checks.json').exists())
                    self.assertFalse((directory/'candidate.json').exists())


if __name__ == '__main__':unittest.main()
