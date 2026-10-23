#!/usr/bin/env python3
"""Validate R1 Windows binary-route metadata; never native/installed acceptance."""
import argparse
import json
from pathlib import Path
import re

from validate_renderer_inventory import _bounded_bytes, _json_bytes, read_json, sha256

CASES={'ordinary':(True,False),'empty':(False,False),'rhi':(True,True),'legacy':(True,False),'dual':(True,True)}
CASES.update({'cpu-python':(True,False),'legacy-python':(True,False),'dual-python':(True,True),'rhi-python':(True,True)})
CASES.update({name+'-no-tests':(False,rhi) for name,(_,rhi) in list(CASES.items()) if name not in ('ordinary','empty')})
HOST_CASES={kind+'-'+profile+suffix:(kind,profile,tests)
    for kind in ('ui','cli','both') for profile in ('legacy','dual','legacy-python','dual-python')
    for tests,suffix in ((True,''),(False,'-no-tests'))}
CASES.update({name:(tests,profile.startswith('dual')) for name,(_,profile,tests) in HOST_CASES.items()})
SETTINGS=dict(os='Windows',arch='x86_64',compiler='msvc',build_type='Release',**{
    'compiler.version':'194','compiler.cppstd':'17','compiler.runtime':'dynamic','compiler.runtime_type':'Release'})
DIRECT={'boost/1.82.0','eigen/3.4.0','lua/5.3.5','rapidjson/1.1.0'}
GTEST='gtest/1.18.0'
LEGACY_DIRECT={'freetype/2.13.0','glew/2.2.0','glm/0.9.9.8'}
CLI_DIRECT={'glfw/3.3.8','libcurl/8.23.0'}
LEGACY={'freetype','glew','glm','opengl','glu','glfw','libcurl','libdxfrw','qt'}


def require(ok,message):
    if not ok:raise ValueError(message)


def case_options(case):
    require(case in CASES,'Unknown R1 Conan case')
    tests,rhi=CASES[case]
    kind,profile,_=HOST_CASES.get(case,('none',case.removesuffix('-no-tests'),tests))
    return dict(with_qt_ui=str(kind in ('ui','both')),with_luacmdinterface=str(kind in ('cli','both')),
        with_pythonscript=str(profile.endswith('-python')),with_legacy_opengl_renderer=str(profile.removesuffix('-python') in ('legacy','dual')),
        with_rendering_unittests='False',with_unittests=str(tests),with_rhi_renderer=str(rhi),with_renderer_tests=str(case=='rhi'))


def validate(graph,lock,case):
    """Conan graph info can exit zero with Missing nodes: inspect every node."""
    require(case in CASES,'Unknown R1 Conan case')
    require(isinstance(graph,dict) and isinstance(graph.get('graph'),dict),'Missing Conan graph')
    data=graph['graph'];nodes=data.get('nodes')
    require(data.get('error','missing') is None and data.get('root')=={'0':''},'Conan graph has an error or foreign consumer')
    require(isinstance(nodes,dict) and 1<len(nodes)<128 and all(isinstance(n,dict) for n in nodes.values()),'Invalid Conan nodes')
    root=nodes.get('0',{})
    require(root.get('recipe')=='Consumer' and root.get('context')=='host' and root.get('settings')==SETTINGS,
            'Conan consumer does not match the declared Windows/MSVC R1 profile')
    tests,rhi=CASES[case]
    kind,profile,_=HOST_CASES.get(case,('none',case.removesuffix('-no-tests'),tests))
    legacy=profile.removesuffix('-python') in ('legacy','dual');ui=kind in ('ui','both');cli=kind in ('cli','both')
    expected=case_options(case)
    require(root.get('options')==expected,'Conan consumer options differ from the selected case')
    dependencies=root.get('dependencies')
    require(isinstance(dependencies,dict) and all(isinstance(d,dict) for d in dependencies.values()),'Missing consumer dependencies')
    direct={d.get('ref') for d in dependencies.values() if d.get('direct') is True}
    require(direct==DIRECT|({GTEST} if tests else set())|(LEGACY_DIRECT if legacy else set())|(CLI_DIRECT if cli else set()),'Missing/unexpected direct Conan requirements')
    require(isinstance(lock,dict) and lock.get('version')=='0.5','Unsupported Conan lock schema')
    locked={}
    for context,key in [('host','requires'),('build','build_requires')]:
        values=lock.get(key)
        require(isinstance(values,list) and all(isinstance(v,str) for v in values),'Missing lock requirements')
        refs=[v.split('%',1)[0] for v in values]
        require(len(refs)==len(set(refs)),'Duplicate lock requirement')
        locked[context]=set(refs)
    require(lock.get('python_requires')==[] and lock.get('config_requires')==[],'Unreviewed Python/config recipe requirements')
    observed={'host':set(),'build':set()};packages=[];skipped={}
    for key,node in nodes.items():
        edges=node.get('dependencies')
        require(isinstance(edges,dict) and all(isinstance(e,dict) and target in nodes and target not in ('0',key) and
                isinstance(nodes[target].get('ref'),str) and e.get('ref')==nodes[target]['ref'].split('#',1)[0] for target,e in edges.items()),
                'Missing/misbound Conan dependency edge')
        if key=='0':continue
        context=node.get('context');ref=node.get('ref');binary=node.get('binary')
        require(context in observed and isinstance(ref,str) and re.fullmatch(r'[a-z0-9_+.-]+/[^/#\s]+#[0-9a-f]{32}',ref),
                'Invalid Conan recipe identity/context')
        require(node.get('rrev')==ref.split('#')[1],'Misbound Conan recipe revision')
        require(not node.get('info_invalid') and not node.get('invalid_build'),'Invalid Conan binary configuration: '+ref)
        if context=='build' and binary=='Skip':
            # Conan can repeat unused tool nodes for different dependency paths.
            # Only identical skipped metadata may repeat; host binaries may not.
            identity={k:node.get(k) for k in ('ref','rrev','context','binary','package_id','prev','settings','options')}
            require(ref not in skipped or skipped[ref]==identity,'Conflicting skipped Conan tool identity')
            skipped[ref]=identity;observed[context].add(ref);continue
        require(ref not in observed[context],'Duplicate Conan recipe identity');observed[context].add(ref)
        require(context=='host' and binary in ('Cache','Download'),'Unavailable/source-built Conan package: '+ref+' ('+str(binary)+')')
        allowed_legacy=({p.split('/')[0] for p in LEGACY_DIRECT}|{'opengl','glu'} if legacy else set())|(
            {p.split('/')[0] for p in CLI_DIRECT} if cli else set())
        require(ref.split('/')[0] not in LEGACY-allowed_legacy,'Unexpected graphics/host dependency: '+ref)
        require(isinstance(node.get('options'),dict) and node['options'].get('shared','False')=='False','R1 core binaries must be static')
        if ref.split('/')[0]=='gtest':
            require(tests,'Tests-OFF route unexpectedly includes GTest')
            require(node['options'].get('build_gmock')=='True' and node['options'].get('no_main')=='False',
                    'GTest binary omits required GMock/main targets')
        require(isinstance(node.get('package_id'),str) and re.fullmatch(r'[0-9a-f]{40}',node['package_id']), 'Missing binary package ID')
        require(isinstance(node.get('prev'),str) and re.fullmatch(r'[0-9a-f]{32}',node['prev']), 'Missing binary package revision')
        require(isinstance(node.get('settings'),dict),'Missing binary settings')
        if binary=='Download':require(node.get('binary_remote')=='conancenter','Unreviewed binary remote')
        packages.append(dict(ref=ref,package_id=node['package_id'],package_revision=node['prev'],settings=dict(node['settings'])))
    require(observed==locked,'Recipe revisions differ from the generated lockfile')
    return dict(case=case,consumer_settings=dict(SETTINGS),consumer_options=expected,packages=sorted(packages,key=lambda p:p['ref']))


def main():
    parser=argparse.ArgumentParser(description=__doc__,allow_abbrev=False)
    parser.add_argument('--graph',type=Path,required=True);parser.add_argument('--lockfile',type=Path,required=True)
    parser.add_argument('--case',choices=sorted(CASES),required=True);parser.add_argument('--report',type=Path,required=True)
    parser.add_argument('--expected-route',type=Path,help='Require the installed graph to retain the preflight binary identities')
    args=parser.parse_args()
    try:
        graph_raw=_bounded_bytes(args.graph,8*1024*1024);lock_raw=_bounded_bytes(args.lockfile,1024*1024)
        result=validate(_json_bytes(graph_raw),_json_bytes(lock_raw),args.case)
        if args.expected_route:
            previous=read_json(args.expected_route)
            require(type(previous.get('schema_version')) is int and previous['schema_version']==1 and
                    previous.get('validator_sha256')==sha256(Path(__file__).read_bytes()) and
                    previous.get('evidence_kind')=='r1-conan-binary-route' and previous.get('phase_or_platform_accepted') is False
                    and previous.get('route')==result,'Installed graph differs from validated binary route')
        record=dict(schema_version=1,evidence_kind='r1-conan-binary-route',phase_or_platform_accepted=False,
            graph_sha256=sha256(graph_raw),lockfile_sha256=sha256(lock_raw),
            validator_sha256=sha256(Path(__file__).read_bytes()),route=result)
        args.report.write_text(json.dumps(record,indent=2)+'\n')
        print('R1 Conan binary metadata: PASS ('+args.case+'); not native Windows acceptance')
    except (OSError,ValueError,KeyError,TypeError) as error:parser.exit(1,'R1 Conan binary metadata: FAIL: '+str(error)+'\n')


if __name__=='__main__':main()
