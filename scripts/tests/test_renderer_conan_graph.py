"""Synthetic Conan graph/lock failures; no package or native Windows proof."""
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import validate_renderer_conan_graph as validator


def fixture(case='ordinary'):
    tests,rhi=validator.CASES[case]
    options=validator.case_options(case);legacy=options['with_legacy_opengl_renderer']=='True';cli=options['with_luacmdinterface']=='True'
    root=dict(recipe='Consumer',context='host',settings=dict(validator.SETTINGS),dependencies={},options=options)
    nodes={'0':root};refs=[]
    for i,name in enumerate(sorted(validator.DIRECT|({validator.GTEST} if tests else set())|(validator.LEGACY_DIRECT if legacy else set())|(validator.CLI_DIRECT if cli else set())),1):
        ref=name+'#'+format(i,'032x');refs.append(ref+'%1.0')
        nodes[str(i)]=dict(ref=ref,name=name.split('/')[0],rrev=ref.split('#')[1],recipe='Cache',binary='Download',context='host',
            package_id=format(i,'040x'),prev=format(i+100,'032x'),binary_remote='conancenter',
            options={'shared':'True' if name.startswith('lua/') else 'False'},
            settings=dict(validator.SETTINGS),dependencies={})
        if name==validator.GTEST:nodes[str(i)]['options'].update(build_gmock='True',no_main='False')
        root['dependencies'][str(i)]={'ref':name,'direct':True}
    return {'graph':dict(root={'0':''},error=None,nodes=nodes)},dict(version='0.5',requires=refs,build_requires=[],python_requires=[],config_requires=[])


class ConanGraphTests(unittest.TestCase):
    def setUp(self):self.graph,self.lock=fixture()
    def test_each_case_requires_exact_profile_options_and_binary_identities(self):
        for case in validator.CASES:
            graph,lock=fixture(case);result=validator.validate(graph,lock,case)
            tests,_=validator.CASES[case];options=validator.case_options(case)
            self.assertEqual(4+int(tests)+3*int(options['with_legacy_opengl_renderer']=='True')+2*int(options['with_luacmdinterface']=='True'),len(result['packages']))
            self.assertEqual(case,result['case'])

    def test_zero_exit_style_missing_graph_is_rejected_despite_no_graph_error(self):
        node=self.graph['graph']['nodes']['1'];node.update(binary='Missing',prev=None)
        with self.assertRaisesRegex(ValueError,'Unavailable/source-built'):validator.validate(self.graph,self.lock,'ordinary')

    def test_tests_off_cannot_hide_gtest_as_a_transitive_dependency(self):
        graph,lock=fixture('rhi-python-no-tests');node=copy.deepcopy(self.graph['graph']['nodes']['1'])
        ref=validator.GTEST+'#'+'a'*32;node.update(ref=ref,rrev='a'*32)
        graph['graph']['nodes']['90']=node
        graph['graph']['nodes']['0']['dependencies']['90']={'ref':validator.GTEST,'direct':False}
        lock['requires'].append(ref+'%1.0')
        with self.assertRaisesRegex(ValueError,'Tests-OFF'):validator.validate(graph,lock,'rhi-python-no-tests')

    def test_explicit_graph_error_status_is_required(self):
        for error in ('missing',{'type':'Invalid'}):
            graph=copy.deepcopy(self.graph)
            if error=='missing':graph['graph'].pop('error')
            else:graph['graph']['error']=error
            with self.assertRaises(ValueError):validator.validate(graph,self.lock,'ordinary')

    def test_legacy_transitive_cannot_hide_behind_a_different_display_name(self):
        node=copy.deepcopy(self.graph['graph']['nodes']['1']);ref='glew/2.2.0#'+'a'*32
        node.update(ref=ref,rrev='a'*32,name='unrelated-display-name')
        self.graph['graph']['nodes']['9']=node;self.graph['graph']['nodes']['0']['dependencies']['9']={'ref':'glew/2.2.0','direct':False}
        self.lock['requires'].append(ref+'%1.0')
        with self.assertRaisesRegex(ValueError,'graphics/host'):validator.validate(self.graph,self.lock,'ordinary')

    def test_build_invalid_editable_and_unknown_binary_states_cannot_pass(self):
        for status in ('Build','Editable','Invalid','Skip',None):
            graph=copy.deepcopy(self.graph);graph['graph']['nodes']['1']['binary']=status
            with self.subTest(status=status),self.assertRaises(ValueError):validator.validate(graph,self.lock,'ordinary')

    def test_system_opengl_and_glu_are_not_cpu_dependencies(self):
        for package in ('opengl','glu'):
            graph,lock=fixture();node=copy.deepcopy(graph['graph']['nodes']['1']);ref=package+'/system#'+'a'*32
            node.update(ref=ref,rrev='a'*32);graph['graph']['nodes']['90']=node
            graph['graph']['nodes']['0']['dependencies']['90']={'ref':package+'/system','direct':False}
            lock['requires'].append(ref+'%1.0')
            with self.subTest(package=package),self.assertRaisesRegex(ValueError,'graphics/host'):validator.validate(graph,lock,'ordinary')

    def test_only_unneeded_build_tools_may_be_skipped(self):
        ref='cmake/4.4.4#'+'a'*32
        self.graph['graph']['nodes']['9']=dict(ref=ref,rrev='a'*32,context='build',binary='Skip',dependencies={})
        self.lock['build_requires']=[ref+'%1.0']
        validator.validate(self.graph,self.lock,'ordinary')
        self.graph['graph']['nodes']['9']['binary']='Download'
        with self.assertRaises(ValueError):validator.validate(self.graph,self.lock,'ordinary')

    def test_root_case_and_windows_profile_cannot_be_relabelled(self):
        for key,value in [('os','Macos'),('arch','armv8'),('compiler.cppstd','14'),('compiler.version','193')]:
            graph=copy.deepcopy(self.graph);graph['graph']['nodes']['0']['settings'][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):validator.validate(graph,self.lock,'ordinary')
        with self.assertRaises(ValueError):validator.validate(self.graph,self.lock,'empty')

    def test_repeated_unused_tool_paths_require_identical_skipped_metadata(self):
        ref='meson/1.3.2#'+'a'*32
        tool=dict(ref=ref,rrev='a'*32,context='build',binary='Skip',dependencies={},settings={},options={})
        self.graph['graph']['nodes'].update({'90':copy.deepcopy(tool),'91':copy.deepcopy(tool)})
        self.lock['build_requires']=[ref+'%1.0']
        validator.validate(self.graph,self.lock,'ordinary')
        self.graph['graph']['nodes']['91']['options']={'changed':'True'}
        with self.assertRaisesRegex(ValueError,'Conflicting skipped'):validator.validate(self.graph,self.lock,'ordinary')

    def test_host_recipe_duplicates_are_rejected_even_if_the_lock_is_unique(self):
        self.graph['graph']['nodes']['90']=copy.deepcopy(self.graph['graph']['nodes']['1'])
        with self.assertRaisesRegex(ValueError,'Duplicate'):validator.validate(self.graph,self.lock,'ordinary')

    def test_missing_duplicate_or_misbound_lock_revisions_rejected(self):
        for mutation in ('missing','duplicate','changed','schema','python'):
            lock=copy.deepcopy(self.lock)
            if mutation=='missing':lock['requires'].pop()
            if mutation=='duplicate':lock['requires']*=2
            if mutation=='changed':lock['requires'][0]=lock['requires'][0].replace('#','#0',1)
            if mutation=='schema':lock['version']='0.6'
            if mutation=='python':lock['python_requires']=['anything/1#'+'a'*32]
            with self.subTest(mutation=mutation),self.assertRaises(ValueError):validator.validate(self.graph,lock,'ordinary')

    def test_missing_nodes_and_foreign_edges_fail_even_with_an_altered_lock(self):
        del self.graph['graph']['nodes']['1'];self.lock['requires'].pop(0)
        with self.assertRaisesRegex(ValueError,'edge'):validator.validate(self.graph,self.lock,'ordinary')

    def test_recipe_binary_revision_remote_and_static_policy_are_required(self):
        for key,value in [('rrev','0'*32),('package_id','bad'),('prev',None),('binary_remote','other'),('options',{'shared':'True'})]:
            graph=copy.deepcopy(self.graph);graph['graph']['nodes']['1'][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):validator.validate(graph,self.lock,'ordinary')
        # The Lua core alone is shared: a static one is refused like a shared other.
        graph=copy.deepcopy(self.graph);node=next(n for n in graph['graph']['nodes'].values() if n.get('name')=='lua')
        node['options']['shared']='False'
        with self.assertRaisesRegex(ValueError,'shared Lua core'):validator.validate(graph,self.lock,'ordinary')
        for key,value in [('build_gmock','False'),('no_main','True')]:
            graph=copy.deepcopy(self.graph);node=next(n for n in graph['graph']['nodes'].values() if n.get('name')=='gtest')
            node['options'][key]=value
            with self.assertRaisesRegex(ValueError,'GMock/main'):validator.validate(graph,self.lock,'ordinary')

    def test_missing_direct_dependency_cannot_be_hidden_in_transitives(self):
        self.graph['graph']['nodes']['0']['dependencies']['1']['direct']=False
        with self.assertRaises(ValueError):validator.validate(self.graph,self.lock,'ordinary')

    def test_output_does_not_mutate_global_profile_settings(self):
        result=validator.validate(self.graph,self.lock,'ordinary');result['consumer_settings']['arch']='other'
        self.assertEqual('x86_64',validator.SETTINGS['arch'])

    def test_cli_reports_metadata_only_and_binds_preflight_binary_route(self):
        with tempfile.TemporaryDirectory(prefix='conan-route-fixture-') as tmp:
            root=Path(tmp);graph=root/'graph.json';lock=root/'case.lock';report=root/'route.json';after=root/'after.json'
            graph.write_text(json.dumps(self.graph));lock.write_text(json.dumps(self.lock))
            command=[sys.executable,'-B',validator.__file__,'--case','ordinary','--graph',str(graph),'--lockfile',str(lock)]
            result=subprocess.run([*command,'--report',str(report)],capture_output=True,timeout=10)
            self.assertEqual(0,result.returncode,result.stderr);data=json.loads(report.read_text())
            self.assertIs(data['phase_or_platform_accepted'],False)
            self.graph['graph']['nodes']['1'].update(binary='Cache',binary_remote=None)
            graph.write_text(json.dumps(self.graph))
            result=subprocess.run([*command,'--report',str(after),'--expected-route',str(report)],capture_output=True,timeout=10)
            self.assertEqual(0,result.returncode,result.stderr)
            self.graph['graph']['nodes']['1']['prev']='f'*32;graph.write_text(json.dumps(self.graph))
            after.unlink()
            result=subprocess.run([*command,'--report',str(after),'--expected-route',str(report)],capture_output=True,timeout=10)
            self.assertEqual(1,result.returncode);self.assertFalse(after.exists())


if __name__=='__main__':unittest.main()
