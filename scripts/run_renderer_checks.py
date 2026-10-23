#!/usr/bin/env python3
"""Run explicit R0 spike or provisional R1 CPU/offscreen contracts, never phase acceptance."""

import argparse
import contextlib
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile
import time
import xml.etree.ElementTree as ET
import zlib
from pathlib import Path
import validate_renderer_inventory as inventory

from validate_renderer_inventory import read_json, sha256, validate_failure, validate_report
from renderer_wayland import WestonSession, validate_input_marker
from renderer_process import capture_display, supervise, validate_display_retired, validate_scope, run_command


VALIDATION_SETTINGS = '''khronos_validation.validate_core = true
khronos_validation.validate_sync = true
khronos_validation.thread_safety = true
khronos_validation.disables =
khronos_validation.message_id_filter =
khronos_validation.debug_action = VK_DBG_LAYER_ACTION_LOG_MSG
khronos_validation.report_flags = info,warn,error
khronos_validation.log_filename = stdout
khronos_validation.enable_message_limit = false
'''


def validation_inputs(args):
    manifest = getattr(args, 'vulkan_validation_manifest', None)
    library = getattr(args, 'vulkan_validation_library', None)
    if manifest is None and library is None:
        return None
    if sys.platform != 'linux' or args.backend != 'vulkan' or not manifest or not library:
        raise ValueError('Vulkan validation requires Linux Vulkan and both manifest/library paths')
    manifest, library = manifest.resolve(strict=True), library.resolve(strict=True)
    layer = read_json(manifest)['layer']
    declared = Path(layer['library_path'])
    resolved = (manifest.parent / declared).resolve()
    if (layer['name'] != 'VK_LAYER_KHRONOS_validation' or layer['type'] != 'GLOBAL' or
            library.name != 'libVkLayer_khronos_validation.so' or
            (resolved != library and (declared.name != str(declared) or declared.name != library.name))):
        raise ValueError('Validation manifest does not identify the declared Khronos library')
    return {'manifest': str(manifest), 'manifest_sha256': sha256(manifest.read_bytes()),
            'library': str(library), 'library_sha256': sha256(library.read_bytes()),
            'api_version': layer['api_version']}


def validation_environment(profile, directory, base):
    # Scope settings to the application, not Weston/Xvfb or the user's Vulkan setup.
    env = {key: value for key, value in base.items()
           if (not key.startswith('VK_') or key in ('VK_DRIVER_FILES', 'VK_ICD_FILENAMES'))
           and key not in ('LD_DEBUG_OUTPUT', 'LD_PRELOAD')}
    library = Path(profile['library'])
    settings = directory / 'vk_layer_settings.txt'
    settings.write_text(VALIDATION_SETTINGS)
    env.update(VK_INSTANCE_LAYERS='VK_LAYER_KHRONOS_validation',
               VK_LAYER_PATH=str(Path(profile['manifest']).parent),
               VK_LAYER_SETTINGS_PATH=str(settings.resolve()), VK_LOADER_LAYERS_DISABLE='~implicit~',
               VK_LOADER_DEBUG='layer,error', LD_DEBUG='libs',
               LD_LIBRARY_PATH=os.pathsep.join(filter(None, (str(library.parent), env.get('LD_LIBRARY_PATH')))))
    return env


def validate_vulkan_validation(profile, directory):
    """Check actual app logs, including teardown; no exit/pixel result overrides a VUID."""
    if profile is None:
        return None
    for name in ('manifest', 'library'):
        if sha256(Path(profile[name]).read_bytes()) != profile[name + '_sha256']:
            raise ValueError('Vulkan validation input changed: ' + name)
    settings = directory / 'vk_layer_settings.txt'
    if settings.read_text() != VALIDATION_SETTINGS:
        raise ValueError('Vulkan validation settings changed')
    logs = {name: (directory / name).read_bytes() for name in ('stdout.log', 'stderr.log')}
    text = '\n'.join(raw.decode(errors='replace') for raw in logs.values())
    errors = re.findall(r'[^\n]*(?:VUID-|SYNC-|Validation Error|ERROR\s*\|)[^\n]*', text, re.IGNORECASE)
    if errors:
        raise ValueError('Vulkan validation error: ' + errors[0][:1000])
    loaded = {str(Path(path.strip()).resolve(strict=True)) for path in
              re.findall(r'calling init:\s+([^\n]*libVkLayer_khronos_validation\.so)', text)}
    instances = re.findall(r'Insert instance layer "VK_LAYER_KHRONOS_validation" \(([^\n]+)\)', text)
    devices = re.findall(r'Inserted device layer "VK_LAYER_KHRONOS_validation" \(([^\n]+)\)', text)
    states = re.findall(r'Khronos Validation Layer Active:\s+Current Enables: ([^\n]*)\s+Current Disables: ([^\n]*)', text)
    if (loaded != {profile['library']} or not instances or not devices or len(states) != len(instances) or
            any(str(Path(path).resolve(strict=True)) != profile['library'] for path in instances + devices) or
            any('VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT' not in enables or disables.strip() != 'None.'
                for enables, disables in states)):
        raise ValueError('Missing/foreign Vulkan validation layer, active synchronization checks or unfiltered core checks')
    return {**profile, 'profile': 'linux-core-sync-v1', 'settings_sha256': sha256(settings.read_bytes()),
            'log_sha256': {name: sha256(raw) for name, raw in logs.items()},
            'instances': len(instances), 'devices': len(devices), 'errors': 0}


def helper_digests():
    return {"runner_sha256": sha256(Path(__file__).read_bytes()),
            "validator_sha256": sha256((Path(__file__).parent / "validate_renderer_inventory.py").read_bytes()),
            "wayland_helper_sha256": sha256((Path(__file__).parent / "renderer_wayland.py").read_bytes()),
            "process_helper_sha256": sha256((Path(__file__).parent / "renderer_process.py").read_bytes())}


def without_launcher(argv):
    result, skip = [], False
    for value in argv:
        if skip:
            skip = False
        elif value == '--xvfb-run':
            skip = True
        elif not value.startswith('--xvfb-run='):
            result.append(value)
    if skip:
        raise ValueError('Missing isolated display launcher')
    return result


def gtest_discovery(data):
    """Parse bounded GoogleTest discovery, preserving every compiled test ID."""
    if len(data) > 2 * 1024 * 1024:
        raise ValueError('Oversized GTest discovery')
    suite, tests = '', []
    for raw in data.decode('utf-8').splitlines():
        line = raw.split('#', 1)[0].rstrip()
        if not line:
            continue
        if re.fullmatch(r'[A-Za-z_][A-Za-z0-9_/]*\.', line):
            suite = line
        elif suite and re.fullmatch(r'  [A-Za-z_][A-Za-z0-9_/]*', line):
            tests.append(suite + line.strip())
        else:
            raise ValueError('Malformed GTest discovery')
    if not tests or len(tests) != len(set(tests)) or any('DISABLED_' in n for n in tests):
        raise ValueError('Empty, duplicate or disabled GTest discovery')
    return sorted(tests)


def gtest_results(data, discovered):
    if len(data) > 8 * 1024 * 1024 or b'<!' in data:
        raise ValueError('Oversized/DTD GTest XML')
    try:
        root = ET.fromstring(data)
    except ET.ParseError as error:
        raise ValueError('Malformed GTest XML') from error
    if (root.tag != 'testsuites' or root.get('tests') != str(len(discovered)) or
            any(root.get(k) != '0' for k in ('failures', 'disabled', 'errors'))):
        raise ValueError('GTest XML totals differ or report failure')
    tests = {}
    for suite in root:
        if (suite.tag != 'testsuite' or suite.get('tests') != str(len(suite.findall('testcase'))) or
                any(suite.get(k) != '0' for k in ('failures', 'disabled', 'skipped', 'errors'))):
            raise ValueError('GTest suite failed, skipped or has inconsistent totals')
        for test in suite.findall('testcase'):
            name = test.get('classname', '') + '.' + test.get('name', '')
            if (name in tests or test.get('classname') != suite.get('name') or
                    test.get('status') != 'run' or test.get('result') != 'completed' or
                    any(test.findall(k) for k in ('failure', 'error', 'skipped'))):
                raise ValueError('Duplicate, failed, skipped or incomplete GTest')
            properties = test.findall('properties/property')
            values = {p.get('name'):p.get('value') for p in properties}
            if len(values) != len(properties) or None in values or None in values.values():
                raise ValueError('Duplicate/invalid GTest properties')
            tests[name] = values
    if sorted(tests) != discovered:
        raise ValueError('GTest discovery/execution ID mismatch')
    return tests


def native_platform():
    native = {'linux':'LINUX', 'darwin':'MAC', 'win32':'WIN'}.get(sys.platform)
    arch = {'aarch64':'ARM64', 'arm64':'ARM64', 'x86_64':'X64', 'amd64':'X64'}.get(platform.machine().lower())
    if not native or not arch:raise ValueError('Unsupported native source platform/architecture')
    if sys.platform == 'darwin':
        import ctypes
        import errno
        value, size = ctypes.c_int(), ctypes.c_size_t(ctypes.sizeof(ctypes.c_int))
        libc = ctypes.CDLL(None,use_errno=True)
        status = libc.sysctlbyname(b'sysctl.proc_translated',ctypes.byref(value),ctypes.byref(size),None,0)
        if (status == 0 and (value.value != 0 or size.value != ctypes.sizeof(value))) or (status != 0 and ctypes.get_errno() != errno.ENOENT):
            raise ValueError('R1 native tuple rejects translated/unknown Mac execution')
    return '{}-{}'.format(native,arch)


def rhi_selection(args):
    selection = inventory.select_case_requirements(inventory.validate_case_manifest(), 'R1', args.configuration)
    config = selection['configuration']
    if (config['artifact'] != 'source' or config['mode'] != 'offscreen' or args.mode != 'offscreen' or
            config['platform'] != native_platform() or
            any(config[k] != getattr(args, arg) for k,arg in
                (('backend','backend'),('device','device'),('qpa','expected_qpa'))) or
            args.device not in ('software','hardware') or not args.build_dir or not args.provider_prefix or
            args.expect_failure or args.scenario != 'standard' or args.background != 'opaque' or
            args.expected_dpr not in (None, 1)):
        raise ValueError('R1 requires a matching native source/offscreen tuple, build and provider; no spike scenarios')
    if args.expected_arch and args.expected_arch.lower() not in (
            ('arm64','aarch64') if config['platform'].endswith('ARM64') else ('x86_64','amd64')):
        raise ValueError('R1 requested architecture differs from the selected native tuple')
    if any(r['binding'] != 'proposed-gtest' or r['dataset'] != 'rhi-build-options' for r in selection['requirements']):
        raise ValueError('R1 selection contains unimplemented execution bindings; no subset may be omitted')
    if args.weston and (sys.platform != 'linux' or args.expected_qpa != 'wayland'):
        raise ValueError('R1 managed compositor requires Linux Wayland')
    return selection


BUILD_INPUT_FLAGS = {'rhi':'WITH_RHI_RENDERER','legacy':'WITH_LEGACY_OPENGL_RENDERER','ui':'WITH_QT_UI',
                     'cli':'WITH_LUACMDINTERFACE','python':'WITH_PYTHONSCRIPT','tests-enabled':'WITH_UNITTESTS'}
BUILD_SOURCE_PATHS = ('CMakeLists.txt','lcviewernoqt/cmake/RendererOptions.cmake','unittest/CMakeLists.txt',
                     'unittest/rendering/buildcontracts.cpp','unittest/rendering/buildcontracts_config.h.in')
BUILD_DISABLED_PACKAGES = ('Qt5','Qt6','OpenGL','GLEW','glew','glfw3','glm','libdxfrw')


def build_selection(args):
    selected=inventory.select_case_requirements(inventory.validate_case_manifest(),'R1',args.configuration)
    config=selected['configuration']
    if (config['platform']!=native_platform() or config['profile']!='cpu' or config['artifact']!='source' or
            any(config[k]!='none' for k in ('backend','device','qpa')) or config['mode']!='cpu' or
            args.app or args.build_dir or args.provider_prefix or args.xvfb_run or args._worker_directory or
            args.weston or args.weston_prefix or args.weston_input_tool or args.icd_manifest or
            args.vulkan_validation_manifest or args.vulkan_validation_library or args.expected_qpa or
            args.backend or args.device or args.mode or args.expect_failure or args.scenario!='standard' or
            args.background!='opaque' or args.expected_dpr is not None):
        raise ValueError('Build contracts require a native source/CPU tuple and fresh owned builds, without graphics arguments')
    if any(r['binding']!='proposed-gtest' or r['dataset']!='cpu-build-options' for r in selected['requirements']):
        raise ValueError('CPU selection contains unimplemented bindings; no subset may be omitted')
    if {r['variant'] for r in selected['requirements']}!={'ordinary','empty','limit'}:
        raise ValueError('CPU build variants changed; implementation review required')
    if args.expected_arch and args.expected_arch.lower() not in (
            ('arm64','aarch64') if config['platform'].endswith('ARM64') else ('x86_64','amd64')):
        raise ValueError('CPU requested architecture mismatch')
    return selected


def build_sdk_options(values):
    result={}
    for value in values:
        key,separator,data=value.partition('=')
        if (not separator or not re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*',key) or key in result or
                key.startswith(('WITH_','LC_CONAN_WITH_','CMAKE_DISABLE_FIND_PACKAGE_','LC_BUILD_CONTRACT')) or
                key in ('TINYSPLINE_BUILD_EXAMPLES','CMAKE_INSTALL_PREFIX','CMAKE_BUILD_TYPE','CMAKE_CONFIGURATION_TYPES',
                        'CMAKE_HOME_DIRECTORY','CMAKE_RUNTIME_OUTPUT_DIRECTORY','CMAKE_LIBRARY_OUTPUT_DIRECTORY','CMAKE_ARCHIVE_OUTPUT_DIRECTORY') or
                '\0' in data or '\n' in data or '\r' in data):
            raise ValueError('Invalid, duplicate or fixture-owned CMake option: '+key)
        result[key]=data
    return result


def build_case_options(row,selected,dataset):
    options=dict(selected['configuration']['expected_options'],WITH_PERSISTENCE=False,WITH_DOCUMENTATION=False,
                 WITH_LIBOPENCAD=False,WITH_COVERAGE=False,TINYSPLINE_BUILD_EXAMPLES=False)
    inputs=dataset[row['variant']]['inputs']
    for key,typed in inputs.items():
        if key not in BUILD_INPUT_FLAGS or typed['type']!='boolean' or typed['unit']!='none' or type(typed['value']) is not bool:
            raise ValueError('Unconsumed/invalid CPU build input')
        options[BUILD_INPUT_FLAGS[key]]=typed['value']
    return options


def build_contract_results(directory,selected):
    discovered=gtest_discovery(inventory._bounded_bytes(directory/'discovery/stdout.log',2*1024*1024))
    tests=gtest_results(inventory._bounded_bytes(directory/'tests/tests.xml',8*1024*1024),discovered)
    required={r['test']:r for r in selected['requirements']}
    if set(tests)!=required.keys():raise ValueError('CPU contract discovery/execution must contain every selected native ID exactly')
    corpus=inventory.read_json(inventory.REPO_ROOT/selected['fixtures']['contracts']['path'])['datasets']['cpu-build-options']
    arch='arm64' if selected['configuration']['platform'].endswith('ARM64') else 'x86_64'
    observations={};compiled=None
    for name,row in required.items():
        props=tests[name]
        if (props.get('build_case')!=row['id'] or props.get('build_fixture_sha256')!=selected['fixtures']['contracts']['sha256'] or
                props.get('build_arch')!=arch or props.get('host_arch')!=arch):
            raise ValueError('CPU case/fixture/architecture identity mismatch')
        source=props.get('build_sources','')
        expected=''.join(sha256((inventory.REPO_ROOT/p).read_bytes())+'  '+p+'\n' for p in BUILD_SOURCE_PATHS)
        if source!=expected or compiled not in (None,source):raise ValueError('CPU test source differs from current compiled inputs')
        compiled=source
        actual=inventory._json_bytes(props.get('build_observations','').encode())
        assertions=corpus[row['variant']]['assertions']
        expected={a['observed']:a['expected']['value'] for a in assertions}
        if json.dumps(actual,sort_keys=True)!=json.dumps(expected,sort_keys=True):raise ValueError('CPU observation violates existing fixture')
        observations[row['id']]=actual
    return dict(configuration_id=selected['configuration_id'],configuration=selected['configuration'],
        manifest_sha256=selected['manifest_sha256'],required_ids=sorted(r['id'] for r in required.values()),
        discovered_ids=discovered,executed_ids=sorted(tests),observations=observations,compiled_sources=compiled)


def build_test_artifact(build):
    reply=build/'.cmake/api/v1/reply';indices=list(reply.glob('index-*.json'))
    if len(indices)!=1:raise ValueError('Missing/ambiguous fresh CMake File API index')
    index=read_json(indices[0]);ref=index['reply']['client-renderer-build']['codemodel-v2']['jsonFile']
    def resolve(name):
        if Path(name).name!=name:raise ValueError('Escaping File API reference')
        return read_json(reply/name)
    model=resolve(ref);configs=[c for c in model['configurations'] if c['name']=='Release']
    if len(configs)!=1:raise ValueError('Missing/ambiguous Release codemodel')
    targets=[r for r in configs[0]['targets'] if r['name']=='lcbuildcontracttest']
    if len(targets)!=1:raise ValueError('CPU build-contract target missing')
    target=resolve(targets[0]['jsonFile'])
    name='lcbuildcontracttest'+('.exe' if sys.platform=='win32' else '')
    artifacts=[(build/a['path']).resolve() for a in target['artifacts'] if Path(a['path']).name==name]
    if len(artifacts)!=1 or not artifacts[0].is_relative_to(build.resolve()):raise ValueError('Foreign/ambiguous CPU test executable')
    return artifacts[0]


def expected_build_rejection(xml, test, diagnostic):
    data=inventory._bounded_bytes(xml,2*1024*1024)
    if re.search(br'<!\s*(?:DOCTYPE|ENTITY)\b',data):raise ValueError('DTD in native rejection XML')
    root=ET.fromstring(data);cases=root.findall('.//testcase');groups=root.findall('testsuite')
    if (root.tag!='testsuites' or len(groups)!=1 or
            any(groups[0].get(k)!=v for k,v in dict(tests='1',failures='1',disabled='0',errors='0',skipped='0').items()) or
            root.get('tests')!='1' or root.get('failures')!='1' or root.get('disabled')!='0' or root.get('errors')!='0' or
            len(cases)!=1 or cases[0].get('classname','')+'.'+cases[0].get('name','')!=test or
            cases[0].get('status')!='run' or cases[0].get('result')!='completed' or root.findall('.//skipped') or
            not cases[0].findall('failure') or diagnostic not in ''.join(''.join(f.itertext()) for f in cases[0].findall('failure'))):
        raise ValueError('Missing/mismatched native build corruption rejection')


def build_corruption_probes(directory,selected,app,step):
    """Controlled mutations of our own artifacts, restored byte-for-byte after each test."""
    by_variant={r['variant']:r['test'] for r in selected['requirements']}
    ordinary=directory/'ordinary';status=read_json(ordinary/'result.json')
    trace=ordinary/('trace.jsonl' if status['trace_format']=='json-v1' else 'stderr.log')
    if status['trace_format']=='json-v1':
        injection=(json.dumps(dict(cmd='find_package',args=['Qt6']))+'\n').encode()
    else:injection=b'contract-fixture.cmake(1):  find_package(Qt6 QUIET )\n'
    reply=directory/'empty/build/.cmake/api/v1/reply'
    index=read_json(next(reply.glob('index-*.json')))
    model=read_json(reply/index['reply']['client-renderer-build']['codemodel-v2']['jsonFile'])
    target=next(t for c in model['configurations'] if c['name']=='Release' for t in c['targets'] if t['name']=='lckernel')
    graph=reply/target['jsonFile'];changed=read_json(graph)
    changed.setdefault('link',{}).setdefault('commandFragments',[]).append(dict(fragment='-lQt6Gui',role='libraries'))
    probes=[('qt-trace','ordinary',trace,trace.read_bytes()+injection,'qt-discovery-count'),
        ('gpu-graph','empty',graph,(json.dumps(changed)+'\n').encode(),'gpu-target-count'),
        ('host-diagnostic','limit',directory/'limit/stderr.log',
         (directory/'limit/stderr.log').read_bytes().replace(b'[no-renderer-for-host]',b'[other-host-error]'),'diagnostic'),
        ('fixture-copy','ordinary',directory/'ordinary/build/unittest/renderer-build-contracts.json',
         (directory/'ordinary/build/unittest/renderer-build-contracts.json').read_bytes()+b' ',
         'Fixture differs from the configured copy')]
    results=[]
    for name,variant,file,mutation,diagnostic in probes:
        raw=file.read_bytes();where=directory/'negative'/name
        where.mkdir(parents=True)
        (where/'original.bin').write_bytes(raw);(where/'mutated.bin').write_bytes(mutation)
        try:
            file.write_bytes(mutation)
            step([str(app),'--gtest_filter='+by_variant[variant],'--gtest_output=xml:'+str(where/'tests.xml')],where,1)
            expected_build_rejection(where/'tests.xml',by_variant[variant],diagnostic)
            results.append(dict(id=name,native_test=by_variant[variant],diagnostic=diagnostic,
                original_sha256=sha256(raw),mutated_sha256=sha256(mutation),xml_sha256=sha256((where/'tests.xml').read_bytes())))
        finally:
            file.write_bytes(raw)
            if file.read_bytes()!=raw:raise ValueError('Failed to restore owned build corruption fixture')
    return results


def run_build_contracts(args):
    selected=build_selection(args);helpers=helper_digests()
    sdk=build_sdk_options(args.cmake_option);overrides={v:{} for v in ('ordinary','empty','limit')}
    for value in args.case_cmake_option:
        variant,separator,option=value.partition(':')
        if not separator or variant not in overrides:raise ValueError('Unknown case-specific SDK option')
        record=build_sdk_options([option]);key=next(iter(record))
        if key in overrides[variant]:raise ValueError('Duplicate case-specific SDK option')
        overrides[variant].update(record)
    cmake=Path(args.cmake or shutil.which('cmake') or '').resolve(strict=True)
    version=subprocess.check_output([str(cmake),'--version'],text=True,timeout=10)
    match=re.match(r'cmake version (\d+)\.(\d+)\.',version)
    if not match or tuple(map(int,match.groups()))<(3,15):raise ValueError('Build cases require the project CMake 3.15 floor')
    trace_format=args.trace_format
    if trace_format=='auto':trace_format='json-v1' if tuple(map(int,match.groups()))>=(3,17) else 'human'
    if trace_format=='json-v1' and tuple(map(int,match.groups()))<(3,17):raise ValueError('JSON trace requires CMake 3.17; use human/auto')
    revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=inventory.REPO_ROOT,text=True).strip()
    if args.expected_revision not in (None,revision):raise ValueError('CPU source revision mismatch')
    corpus=read_json(inventory.REPO_ROOT/selected['fixtures']['contracts']['path'])['datasets']['cpu-build-options']
    sources={p:sha256((inventory.REPO_ROOT/p).read_bytes()) for p in BUILD_SOURCE_PATHS}
    args.report_dir.mkdir(parents=True,exist_ok=True)
    directory=Path(tempfile.mkdtemp(prefix='build-',dir=args.report_dir)).resolve()
    deadline=time.monotonic()+args.timeout;commands=[];records={}
    env={k:v for k,v in os.environ.items() if not k.startswith('GTEST_') and k not in
         ('DISPLAY','XAUTHORITY','WAYLAND_DISPLAY','WAYLAND_SOCKET')}
    def step(command,where,expected=0):
        where.mkdir(parents=True,exist_ok=True)
        remaining=deadline-time.monotonic()
        if remaining<=0:raise ValueError('CPU build-contract deadline expired; logs in '+str(directory))
        record=run_command(command,where,remaining,env,expected_exit=expected)
        commands.append(record);return record
    for row in sorted(selected['requirements'],key=lambda r:r['variant']):
        variant=row['variant'];case=directory/variant;build=case/'build'
        query=build/'.cmake/api/v1/query/client-renderer-build';query.mkdir(parents=True)
        (query/'codemodel-v2').touch()
        options=build_case_options(row,selected,corpus)
        command=[str(cmake),'-S',str(inventory.REPO_ROOT),'-B',str(build),'-DCMAKE_BUILD_TYPE=Release',
            '-DCMAKE_CONFIGURATION_TYPES=Release','-DCMAKE_INSTALL_PREFIX='+str(case/'stage'),
            '--trace-expand']
        if trace_format=='json-v1':command+=['--trace-format=json-v1','--trace-redirect='+str(case/'trace.jsonl')]
        if args.generator:command+=['-G',args.generator]
        command+=['-D'+k+'='+v for k,v in dict(sdk,**overrides[variant]).items()]
        command+=['-D'+k+':BOOL='+('ON' if v else 'OFF') for k,v in options.items()]
        command+=['-DCMAKE_DISABLE_FIND_PACKAGE_'+p+'=ON' for p in BUILD_DISABLED_PACKAGES]
        record=step(command,case,1 if row['expect']=='diagnostic' else 0)
        records[variant]=dict(exit_code=record['launcher_exit_status'],trace_format=trace_format)
        (case/'result.json').write_text(json.dumps(records[variant],indent=2)+'\n')
    ordinary=directory/'ordinary/build'
    step([str(cmake),'--build',str(ordinary),'--config','Release','--target','lcbuildcontracttest','--parallel','4'],directory/'compile')
    app=build_test_artifact(ordinary);binary=sha256(app.read_bytes());env['LC_BUILD_CONTRACT_RUN']=str(directory)
    step([str(app),'--gtest_list_tests'],directory/'discovery')
    found=gtest_discovery((directory/'discovery/stdout.log').read_bytes())
    if set(found)!={r['test'] for r in selected['requirements']}:raise ValueError('Native build-contract suite missing or changed')
    testdir=directory/'tests'
    step([str(app),'--gtest_output=xml:'+str(testdir/'tests.xml')],testdir)
    result=build_contract_results(directory,selected)
    rejections=build_corruption_probes(directory,selected,app,step)
    if build_contract_results(directory,selected)!=result:raise ValueError('Positive build evidence changed after corruption probes')
    if (helper_digests()!=helpers or build_selection(args)!=selected or sha256(app.read_bytes())!=binary or
            {p:sha256((inventory.REPO_ROOT/p).read_bytes()) for p in sources}!=sources):
        raise ValueError('CPU build inputs/helper/binary changed during execution')
    artifacts={str(p.relative_to(directory)):sha256(p.read_bytes()) for p in directory.rglob('*') if p.is_file() and
        (p.suffix in ('.json','.jsonl','.log','.xml') or p.name=='CMakeCache.txt') and not p.is_symlink()}
    evidence=dict(schema_version=1,evidence_kind='r1-cpu-build-contract-runner',phase_or_platform_accepted=False,
        **helpers,build_contracts=result,expected_rejections=rejections,source_revision=revision,source_sha256=sources,commands=commands,
        app=str(app),app_sha256=binary,cmake=str(cmake),cmake_sha256=sha256(cmake.read_bytes()),
        cmake_version=version.strip(),trace_format=trace_format,
        host=dict(system=platform.system(),release=platform.release(),machine=platform.machine()),artifacts_sha256=artifacts)
    (directory/'checks.json').write_text(json.dumps(evidence,indent=2)+'\n')
    print('R1 CPU build contracts: PASS ({} native IDs); {}'.format(len(found),directory))
    return directory


def source_worker_argv(argv,args):
    result=[];skip=False
    for value in argv:
        if skip:skip=False;continue
        if value in ('--build-source','--library-matrix'):continue
        if value in ('--cmake','--generator','--cmake-option','--case-cmake-option'):skip=True;continue
        if value.startswith(('--cmake=','--generator=','--cmake-option=','--case-cmake-option=')):continue
        result.append(value)
    if skip:raise ValueError('Missing source-build argument value')
    return result+['--app',str(args.app),'--build-dir',str(args.build_dir)]


def source_targets(build):
    """Read the owned Release File API graph, never infer targets from filenames."""
    reply=build/'.cmake/api/v1/reply';indices=list(reply.glob('index-*.json'))
    if len(indices)!=1:raise ValueError('Missing/ambiguous source File API index')
    def document(ref):
        name=ref['jsonFile']
        if not isinstance(name,str) or Path(name).name!=name:raise ValueError('Escaping source File API reference')
        return read_json(reply/name)
    index=read_json(indices[0]);model=document(index['reply']['client-renderer-source']['codemodel-v2'])
    if (model['kind']!='codemodel' or model['version']['major']!=2 or
            Path(model['paths']['source']).resolve()!=inventory.REPO_ROOT or Path(model['paths']['build']).resolve()!=build.resolve()):
        raise ValueError('Foreign source File API model')
    configs=[c for c in model['configurations'] if c['name']=='Release']
    if len(configs)!=1:raise ValueError('Missing/ambiguous Release source configuration')
    targets={};identities=set()
    for ref in configs[0]['targets']:
        item=document(ref);name=item['name']
        identifier=item.get('id')
        if (name!=ref['name'] or not isinstance(identifier,str) or not identifier or
                identifier!=ref.get('id') or identifier in identities):
            raise ValueError('Duplicate/misbound source target')
        identities.add(identifier)
        # Visual Studio emits a distinct ALL_BUILD utility for each project.
        # Ignore only identified generator utilities, never application targets.
        if item.get('isGeneratorProvided'):
            if (name not in ('ALL_BUILD','ZERO_CHECK','INSTALL','RUN_TESTS','PACKAGE') or
                    item.get('type')!='UTILITY' or item.get('artifacts') or item.get('compileGroups')):
                raise ValueError('Unexpected generator-provided source target')
            continue
        if name in targets or name in ('ALL_BUILD','ZERO_CHECK','INSTALL','RUN_TESTS','PACKAGE'):
            raise ValueError('Duplicate/misbound source target')
        targets[name]=item
    return targets


CPU_PUBLIC_HEADERS=('painters/lcpainter.h','painters/createpainter.h','painters/renderframe.h','documentcanvas.h','lcdrawoptions.h')
CPU_HEADER_TEMPLATE='unittest/lcviewernoqt/cpu_headercheck.cpp.in'


def cpu_header_inputs():
    paths=[CPU_HEADER_TEMPLATE]+['lcviewernoqt/'+h for h in CPU_PUBLIC_HEADERS]
    return {p:sha256((inventory.REPO_ROOT/p).read_bytes()) for p in paths}


def cpu_header_contract(build,compiled=False):
    """Verify actual independent C++14 consumers, not a target name alone."""
    target=source_targets(build).get('lcviewer_cpu_headercheck',{})
    groups=target.get('compileGroups',[])
    if (target.get('type')!='OBJECT_LIBRARY' or not groups or
            any(g.get('language')!='CXX' or g.get('languageStandard',{}).get('standard')!='14' for g in groups)):
        raise ValueError('Missing independent C++14 CPU header target')
    template=(inventory.REPO_ROOT/CPU_HEADER_TEMPLATE).read_text()
    directory=build/'lcviewernoqt/cpu-headers';sources={}
    for header in CPU_PUBLIC_HEADERS:
        path=directory/(re.sub(r'[^A-Za-z0-9_]','_',header)+'.cpp')
        if path.read_text()!=template.replace('@LC_CPU_HEADER@',header):raise ValueError('Changed configured CPU header consumer')
        sources[str(path.resolve())]=sha256(path.read_bytes())
    actual=set()
    for source in target.get('sources',[]):
        if 'compileGroupIndex' not in source:continue
        if type(source['compileGroupIndex']) is not int or not 0<=source['compileGroupIndex']<len(groups):
            raise ValueError('Invalid CPU header compile group')
        path=Path(source['path']);name=str((path if path.is_absolute() else inventory.REPO_ROOT/path).resolve())
        if name in actual:raise ValueError('Duplicate CPU header source')
        actual.add(name)
    if actual!=sources.keys():raise ValueError('Missing/foreign independent CPU header source')
    outputs=inventory._bounded_bytes(directory/'objects-Release.txt',64*1024).decode().splitlines()
    if len(outputs)!=len(sources) or len(set(outputs))!=len(outputs):raise ValueError('Missing/duplicate CPU header object list')
    objects={};seen=set()
    for name in outputs:
        path=Path(name)
        if not path.is_absolute() or not path.resolve().is_relative_to(build.resolve()) or path.suffix not in ('.o','.obj'):
            raise ValueError('Foreign CPU header object')
        if path.resolve() in seen:raise ValueError('Duplicate resolved CPU header object')
        seen.add(path.resolve())
        if compiled:
            if not path.is_file() or path.stat().st_size==0:raise ValueError('CPU header object was not compiled')
            objects[str(path.resolve().relative_to(build.resolve()))]=sha256(path.read_bytes())
    return dict(standard='14',template_sha256=sha256((inventory.REPO_ROOT/CPU_HEADER_TEMPLATE).read_bytes()),
        headers_sha256={h:sha256((inventory.REPO_ROOT/'lcviewernoqt'/h).read_bytes()) for h in CPU_PUBLIC_HEADERS},
        configured_sources_sha256=sources,objects_sha256=objects)


PYTHON_TEST_SOURCES=('python/pythonbindings_test.cpp','python/demoscript_test.cpp','python/parity_test.cpp',
    'python/python_runtime_test.cpp','scripting/scriptcallback_test.cpp','scripting/scriptobject_test.cpp',
    'scripting/eventbus_test.cpp','scripting/customentitymanager_test.cpp')
LIBRARY_PROFILES={'cpu':(False,False,False),'legacy':(True,False,False),'dual':(True,True,False),
    'cpu-python':(False,False,True),'legacy-python':(True,False,True),
    'dual-python':(True,True,True),'rhi-python':(False,True,True)}
HOST_PROFILES=('legacy','dual','legacy-python','dual-python')
UI_TEST_SOURCES=tuple('ui/'+name+'.cpp' for name in (
    'testtoolbar','testscriptdock','uitests','testlayers','testlinepatterns','testcommandline',
    'testcolorselect','testlinewidthselect','testmainwindow','testwindowmanager','gui/testmenuitem',
    'gui/testmenu','gui/testtoolbartab','gui/testtoolbargroup','gui/testtoolbarbutton',
    'gui/testinputguiwidgets','gui/testdialogwidget','gui/testpyguibridge'))


def library_profile_graph(build,flags,hosts=False):
    ui=hosts and flags['WITH_QT_UI'];cli=hosts and flags['WITH_LUACMDINTERFACE']
    if hosts and (not (ui or cli) or not flags['WITH_LEGACY_OPENGL_RENDERER'] or
                  (ui and not flags['WITH_PERSISTENCE'])):
        raise ValueError('Host checks require a real legacy host; UI also requires persistence')
    preflight=read_json(build/'renderer-preflight.json')
    if preflight.get('evidence_kind')!='renderer-option-preflight':raise ValueError('Missing effective renderer options')
    for key in ('WITH_RHI_RENDERER','WITH_LEGACY_OPENGL_RENDERER','WITH_QT_UI','WITH_LUACMDINTERFACE',
                'WITH_PYTHONSCRIPT','WITH_UNITTESTS','WITH_RENDERER_TESTS','WITH_RENDERING_UNITTESTS','WITH_CAIRO','WITH_PERSISTENCE'):
        if preflight['options'].get(key) is not flags[key]:raise ValueError('Effective source option mismatch: '+key)
    cache=(build/'CMakeCache.txt').read_text()
    for key,value in flags.items():
        if not re.search(r'^'+key+':BOOL='+('ON' if value else 'OFF')+r'$',cache,re.M):raise ValueError('Source cache option mismatch: '+key)
    targets=source_targets(build)
    expected={'lcviewer_opengl':flags['WITH_LEGACY_OPENGL_RENDERER'],'lcrenderer_rhi':flags['WITH_RHI_RENDERER'],
              'lcunittest':flags['WITH_UNITTESTS'],'lcluascript':True,'lcscripting':True,'lcviewernoqt':True,'lckernel':True,
              'lcviewer_cpu_headercheck':flags['WITH_UNITTESTS'],'lcbuildcontracttest':flags['WITH_UNITTESTS'],
              'librecad':ui,'lcui':ui and flags['WITH_UNITTESTS'],'lcui_objects':ui,
              'luacmdinterface':cli,'persistence':flags['WITH_PERSISTENCE'],
              'lcpythonscript':flags['WITH_PYTHONSCRIPT'],'lcuipy':False,
              'lcuipy_build_tree':flags['WITH_PYTHONSCRIPT'],
              'lcrenderertest':False,'lcrenderer_headercheck':False}
    for key,present in expected.items():
        if (key in targets)!=present:raise ValueError('Library matrix target mismatch: '+key)
    def sources(name):return [s['path'].replace('\\','/') for s in targets.get(name,{}).get('sources',[])]
    factory='painters/opengl/createpainter.cpp'
    if any(p.endswith(factory) for p in sources('lcviewernoqt')):raise ValueError('Legacy factory leaked into CPU target')
    if flags['WITH_LEGACY_OPENGL_RENDERER'] and not any(p.endswith(factory) for p in sources('lcviewer_opengl')):
        raise ValueError('Legacy library omits its real factory')
    for name in PYTHON_TEST_SOURCES:
        if any(p.endswith(name) for p in sources('lcunittest'))!=(flags['WITH_PYTHONSCRIPT'] and flags['WITH_UNITTESTS']):
            raise ValueError('Python test-source membership mismatch: '+name)
    if hosts:
        test_sources={name:ui for name in UI_TEST_SOURCES}
        test_sources.update({'persistence/dxfexporttest.cpp':flags['WITH_PERSISTENCE'],
            'lcviewernoqt/testcairofactory.cpp':flags['WITH_CAIRO'],
            'python/persistence_test.cpp':flags['WITH_PYTHONSCRIPT'] and flags['WITH_PERSISTENCE']})
        for name,enabled in test_sources.items():
            if any(p.endswith(name) for p in sources('lcunittest'))!=(enabled and flags['WITH_UNITTESTS']):
                raise ValueError('Host test-source membership mismatch: '+name)
    trace=inventory._bounded_bytes(build.parent/'trace.jsonl',64*1024*1024).splitlines()
    if not trace or inventory._json_bytes(trace[0]).get('version',{}).get('major')!=1:raise ValueError('Missing library configure trace')
    packages=[];cairo=False
    for line in trace[1:]:
        record=inventory._json_bytes(line)
        if record.get('cmd','').lower()=='find_package':packages.append(record['args'][0].lower())
        if record.get('cmd','').lower()=='pkg_check_modules' and record['args'][:2]==['CAIRO','REQUIRED']:
            cairo=any(a.startswith('cairo') for a in record['args'][2:])
    if not packages:raise ValueError('Empty library package-discovery trace')
    forbidden=set()
    if not flags['WITH_PERSISTENCE']:forbidden.add('libdxfrw')
    if not cli:forbidden.update(('glfw','glfw3','curl'))
    forbidden.update(p for p in packages if p.startswith('qt5'))
    if not flags['WITH_UNITTESTS']:forbidden.update(('gtest','gmock','googletest'))
    if not flags['WITH_PYTHONSCRIPT']:forbidden.update(p for p in packages if p.startswith('python'))
    elif 'python' not in packages:raise ValueError('Missing selected Python discovery')
    if not (flags['WITH_RHI_RENDERER'] or ui):forbidden.update(p for p in packages if p.startswith('qt6'))
    if not flags['WITH_LEGACY_OPENGL_RENDERER']:forbidden.update(('glew','glfw','glfw3','glm'))
    if not flags['WITH_RHI_RENDERER'] and not flags['WITH_LEGACY_OPENGL_RENDERER']:forbidden.add('opengl')
    if set(packages)&forbidden:raise ValueError('Forbidden library discovery: '+str(sorted(set(packages)&forbidden)))
    if hosts:
        required=({'qt6'} if ui or flags['WITH_RHI_RENDERER'] else set())|(
            {'libdxfrw'} if flags['WITH_PERSISTENCE'] else set())|({'glfw3','curl'} if cli else set())
        if not required<=set(packages) or cairo!=flags['WITH_CAIRO']:
            raise ValueError('Missing actual host dependency discovery')
    provider=None;path=build/'python-runtime-inventory.txt'
    if flags['WITH_PYTHONSCRIPT']:
        raw=inventory._bounded_bytes(path,64*1024);provider={}
        for line in raw.decode().splitlines():
            key,separator,value=line.partition('=')
            if not separator or key in provider:raise ValueError('Malformed Python provider inventory')
            provider[key]=value
        if provider.get('provider_implementation')!='CPython':raise ValueError('Missing CPython provider identity')
        if provider.get('target_pointer_bits')!='64' or not provider.get('provider_version') or not provider.get('provider_soabi'):
            raise ValueError('Missing Python provider ABI')
        for key in ('executable','link_library'):
            selected=Path(provider[key])
            if not selected.is_absolute() or not selected.is_file():raise ValueError('Missing Python provider file: '+key)
            provider[key+'_sha256']=sha256(selected.read_bytes())
        provider['headers_sha256']={str(Path(p)/'Python.h'):sha256((Path(p)/'Python.h').read_bytes()) for p in provider['include_dirs'].split(';')}
        provider['inventory_sha256']=sha256(raw)
    elif path.exists():raise ValueError('Python OFF produced a runtime inventory')
    host_sdk={}
    if hosts:
        def entry(name):
            values=re.findall(r'^'+name+r':[^=\n]+=([^\n]+)$',cache,re.M)
            if len(values)!=1 or not Path(values[0]).is_absolute():raise ValueError('Missing host SDK cache identity: '+name)
            return Path(values[0]).resolve(strict=True)
        if flags['WITH_PERSISTENCE']:
            include=entry('LIBDXFRW_INCLUDE_DIR');library=entry('LIBDXFRW_LIBRARY')
            headers=sorted(include.rglob('*.h'))
            if not (include/'libdxfrw.h').is_file() or not headers or not library.is_file() or not library.stat().st_size:
                raise ValueError('Missing selected libdxfrw SDK files')
            for selected in [library,*headers]:host_sdk[str(selected)]=sha256(selected.read_bytes())
        modules=(['Qt6Core','Qt6Gui'] if ui or flags['WITH_RHI_RENDERER'] else [])+(
            ['Qt6Widgets','Qt6UiTools','Qt6OpenGL','Qt6OpenGLWidgets','Qt6Svg'] if ui else [])+(
            ['Qt6ShaderTools'] if flags['WITH_RHI_RENDERER'] else [])
        for name in modules:
            selected=entry(name+'_DIR')/(name+'Config.cmake')
            host_sdk[str(selected)]=sha256(selected.read_bytes())
    return dict(targets=sorted(targets),package_discoveries=packages,python_provider=provider,
                host_sdk_files_sha256=host_sdk,
                cpu_headers=cpu_header_contract(build) if flags['WITH_UNITTESTS'] else None,
                legacy_factory_owner='lcviewer_opengl' if flags['WITH_LEGACY_OPENGL_RENDERER'] else None)


def library_artifacts(build,names,types=('STATIC_LIBRARY','SHARED_LIBRARY')):
    """Hash actual owned library outputs named by the Release File API graph."""
    targets=source_targets(build);result={};seen=set()
    for name in names:
        target=targets[name];outputs=target.get('artifacts',[])
        if target.get('type') not in types or not outputs:
            raise ValueError('Missing production library artifacts: '+name)
        result[name]={}
        for item in outputs:
            path=Path(item['path']);path=(path if path.is_absolute() else build/path).resolve()
            if not path.is_relative_to(build.resolve()) or path in seen:
                raise ValueError('Foreign/duplicate/uncompiled production library artifact: '+name)
            seen.add(path)
            # MSVC advertises a PDB even for Release configurations without
            # debug-information flags. Actual DLL/import/object outputs remain
            # mandatory, and any emitted PDB is fingerprinted too.
            if sys.platform=='win32' and path.suffix.lower()=='.pdb' and not path.exists():continue
            if not path.is_file() or path.stat().st_size==0:
                raise ValueError('Foreign/duplicate/uncompiled production library artifact: '+name)
            result[name][str(path.relative_to(build.resolve()))]=sha256(path.read_bytes())
        if not result[name]:raise ValueError('Missing production library artifacts: '+name)
    return result


def build_library_matrix(args,selected,sdk,parent,tests=True):
    profiles=dict(LIBRARY_PROFILES)
    if not tests:profiles['rhi']=(False,True,False)
    overrides={p:{} for p in profiles}
    for value in args.case_cmake_option:
        profile,separator,option=value.partition(':')
        if not separator or profile not in profiles:raise ValueError('Unknown source library SDK profile')
        data=build_sdk_options([option]);key=next(iter(data))
        if key in overrides[profile]:raise ValueError('Duplicate source library SDK option')
        overrides[profile].update(data)
    records=[];common_ids={};python_provider=None
    env={k:v for k,v in os.environ.items() if not k.startswith('GTEST_') and k not in ('DISPLAY','XAUTHORITY','WAYLAND_DISPLAY','WAYLAND_SOCKET')}
    for profile,(legacy,rhi,python) in profiles.items():
        directory=parent/'library-matrix'/profile;build=directory/'build'
        query=build/'.cmake/api/v1/query/client-renderer-source';query.mkdir(parents=True);(query/'codemodel-v2').touch()
        flags=dict(selected['configuration']['expected_options'],WITH_LEGACY_OPENGL_RENDERER=legacy,WITH_RHI_RENDERER=rhi,WITH_PYTHONSCRIPT=python,
            WITH_UNITTESTS=tests,WITH_RENDERER_TESTS=False,WITH_PERSISTENCE=False,WITH_LIBOPENCAD=False,WITH_DOCUMENTATION=False,
            WITH_COVERAGE=False,TINYSPLINE_BUILD_EXAMPLES=False)
        deadline=time.monotonic()+args.timeout;commands=[]
        def step(command,name):
            where=directory/name;where.mkdir();remaining=deadline-time.monotonic()
            if remaining<=0:raise ValueError('Library build deadline expired; logs in '+str(directory))
            commands.append(run_command(command,where,remaining,env))
        command=[str(Path(args.cmake or shutil.which('cmake')).resolve(strict=True)),'-S',str(inventory.REPO_ROOT),'-B',str(build),
            '-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CONFIGURATION_TYPES=Release','-DCMAKE_INSTALL_PREFIX='+str(directory/'stage'),
            '--trace-expand','--trace-format=json-v1','--trace-redirect='+str(directory/'trace.jsonl')]
        if args.generator:command+=['-G',args.generator]
        selected_sdk=dict(sdk,**overrides[profile])
        command+=['-D'+k+'='+v for k,v in selected_sdk.items()]
        command+=['-D'+k+':BOOL='+('ON' if v else 'OFF') for k,v in flags.items()]
        forbidden=['libdxfrw','Qt5']+(['Qt6'] if not rhi else [])+(['GLEW','glew','glfw3','glm'] if not legacy else [])
        if not tests:forbidden+=['GTest','GMock','googletest']
        if not python:forbidden+=['Python','Python3','PythonInterp','PythonLibs']
        if not rhi and not legacy:forbidden+=['OpenGL']
        command+=['-DCMAKE_DISABLE_FIND_PACKAGE_'+p+'=ON' for p in forbidden]
        step(command,'configure');observed=library_profile_graph(build,flags)
        if python and 'Python_EXECUTABLE' in selected_sdk and Path(selected_sdk['Python_EXECUTABLE']).resolve()!=Path(observed['python_provider']['executable']).resolve():
            raise ValueError('Configured Python differs from requested SDK')
        targets=(['lcunittest','lcluascript','lcviewer_cpu_headercheck'] if tests else
                 ['lckernel','lcviewernoqt','lcscripting','lcluascript']+(['lcpythonscript'] if python else []))
        targets+=(['lcviewer_opengl'] if legacy else [])+(['lcrenderer_rhi'] if rhi else [])
        step([command[0],'--build',str(build),'--config','Release','--target',*targets,'--parallel','3'],'compile')
        if not tests:
            artifacts=library_artifacts(build,targets)
            if library_profile_graph(build,flags)!=observed:raise ValueError('Production build evidence changed')
            if python:
                if python_provider not in (None,observed['python_provider']):raise ValueError('Python profiles selected different providers')
                python_provider=observed['python_provider']
            record=dict(schema_version=1,evidence_kind='r1-library-compile-only',phase_or_platform_accepted=False,
                runtime_tests_executed=False,profile=profile,options=flags,observed=observed,commands=commands,
                artifacts_sha256=artifacts)
            (directory/'library-build.json').write_text(json.dumps(record,indent=2)+'\n');records.append(record)
            continue
        app=build/'bin'/('lcunittest'+('.exe' if sys.platform=='win32' else ''));binary=sha256(app.read_bytes())
        step([str(app),'--gtest_list_tests'],'cpu-discovery')
        listing=re.sub(br'^LibreCAD v[^\r\n]+\r?\n',b'',(directory/'cpu-discovery/stdout.log').read_bytes(),count=1)
        found=gtest_discovery(listing);xml=directory/'cpu-tests/tests.xml'
        step([str(app),'--gtest_output=xml:'+str(xml)],'cpu-tests');gtest_results(xml.read_bytes(),found)
        if python in common_ids and common_ids[python]!=found:raise ValueError('Library profiles compiled out different CPU/Python IDs')
        common_ids[python]=found
        python_ids=sorted(set(found)-set(common_ids[False])) if python else []
        if python:
            if not set(common_ids[False])<set(found):raise ValueError('Python build lost CPU IDs or omitted Python tests')
            if not {'PythonRuntime.IsolatedProviderIgnoresAmbientPaths','PythonRuntime.RuntimeConfigurationIsValidatedInFreshProcesses'}<=set(python_ids):
                raise ValueError('Python build omitted selected-provider runtime probes')
            if python_provider not in (None,observed['python_provider']):raise ValueError('Python profiles selected different providers')
            python_provider=observed['python_provider']
        if sha256(app.read_bytes())!=binary or library_profile_graph(build,flags)!=observed:raise ValueError('Library build evidence changed')
        record=dict(schema_version=1,evidence_kind='r1-library-option-build',phase_or_platform_accepted=False,
            profile=profile,options=flags,observed=observed,commands=commands,cpu_ids=found,
            base_cpu_ids=common_ids[False],python_ids=python_ids,app_sha256=binary,cpu_headers=cpu_header_contract(build,True))
        (directory/'library-build.json').write_text(json.dumps(record,indent=2)+'\n');records.append(record)
    return records


def production_inputs(hosts=False):
    paths=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','-z'],
        cwd=inventory.REPO_ROOT).decode().split('\0')
    return {p:sha256((inventory.REPO_ROOT/p).read_bytes()) for p in sorted(set(paths)) if p and
        (Path(p).suffix in ('.c','.cpp','.h','.hpp','.in','.cmake','.vert','.frag') or Path(p).name in ('CMakeLists.txt','conanfile.py')
         or (hosts and p.startswith(('lcUI/','res/','lcUIPy/'))))
        and (inventory.REPO_ROOT/p).is_file()}


def host_object_graph(build,flags):
    """Audit actual File API objects, independent of Make/Ninja/MSVC syntax."""
    targets=source_targets(build);objects=targets.get('lcui_objects',{});compiled=[];outputs=[]
    if not flags['WITH_QT_UI'] and any(n in targets for n in ('lcui_objects','lcui','librecad')):
        raise ValueError('CLI-only host graph contains UI targets')
    if flags['WITH_QT_UI']:
        groups=objects.get('compileGroups',[])
        if objects.get('type')!='OBJECT_LIBRARY' or not groups or any(
                g.get('language')!='CXX' or int(g.get('languageStandard',{}).get('standard','0'))<17 or
                (any(d['define'].split('=')[0]=='LC_WITH_PYTHONSCRIPT' for d in g.get('defines',[]))
                 !=flags['WITH_PYTHONSCRIPT']) for g in groups):
            raise ValueError('Missing UI object C++17/Python compile requirements')
        if flags['WITH_PYTHONSCRIPT'] and targets['lcuipy_build_tree']['id'] not in {
                d['id'] for d in objects.get('dependencies',[])}:
            raise ValueError('UI compilation lacks Python module-generation dependency')
        compiled=[s for s in objects.get('sources',[]) if 'compileGroupIndex' in s]
        names=[Path(s['path']).name for s in compiled]
        inputs=[(Path(s['path']) if Path(s['path']).is_absolute() else inventory.REPO_ROOT/s['path']).resolve() for s in compiled]
        outputs=[(build/a['path']).resolve() for a in objects.get('artifacts',[])]
        if (len(outputs)!=len(compiled) or len(set(outputs))!=len(outputs) or
                len(set(inputs))!=len(inputs) or any(not (p.is_relative_to(inventory.REPO_ROOT) or p.is_relative_to(build.resolve())) for p in inputs) or
                any(not p.is_relative_to(build.resolve()) for p in outputs) or
                sum(n in ('mocs_compilation.cpp','mocs_compilation_Release.cpp') for n in names)!=1 or names.count('qrc_resource.cpp')!=1):
            raise ValueError('Missing, duplicated or foreign compiled UI/Qt objects')
    hosts=(['librecad']+(['lcui'] if flags['WITH_UNITTESTS'] else []) if flags['WITH_QT_UI'] else [])+(
        ['luacmdinterface'] if flags['WITH_LUACMDINTERFACE'] else [])
    for name in hosts:
        target=targets.get(name,{})
        if target.get('type')!=('SHARED_LIBRARY' if name=='lcui' else 'EXECUTABLE'):
            raise ValueError('Wrong host artifact type: '+name)
        deps={d['id'] for d in target.get('dependencies',[])}
        if targets['lcviewer_opengl']['id'] not in deps or (flags['WITH_RHI_RENDERER'] and targets['lcrenderer_rhi']['id'] in deps):
            raise ValueError('Host must retain its legacy, not RHI, factory dependency')
        if name=='luacmdinterface':continue
        # File API source paths are relative to the top-level source tree,
        # including generated objects when CI builds inside the checkout.
        actual=[(inventory.REPO_ROOT/s['path'].replace('$(Configuration)','Release')).resolve()
            for s in target.get('sources',[]) if Path(s['path']).suffix.lower() in ('.o','.obj')]
        if (objects['id'] not in deps or len(actual)!=len(outputs) or set(actual)!=set(outputs) or
                any('compileGroupIndex' in s for s in target.get('sources',[])) or name+'_autogen' in targets):
            raise ValueError('Host recompiles or omits shared UI objects: '+name)
    for name in ('lckernel','lcviewernoqt'):
        groups=[g for g in targets[name].get('compileGroups',[]) if g['language']=='CXX']
        if not groups or any(g.get('languageStandard',{}).get('standard')!='14' for g in groups):
            raise ValueError('UI requirements leaked into CPU C++14 targets')
    return dict(ui_compilations=len(compiled),shared_object_paths=sorted(str(p.relative_to(build.resolve())) for p in outputs))


def host_test_selection(raw,discovered,inputs):
    """Bind every compiled list-only GTest ID to its owned source/header origin.

    Native discovery handles conditional/typed/parameterized registrations; it is
    never execution proof. Text and JSON discovery must describe the same IDs.
    """
    if len(raw)>8*1024*1024:raise ValueError('Oversized compiled GTest discovery metadata')
    data=inventory._json_bytes(raw);origins={};widget=[];required=[];lines={}
    if not isinstance(data,dict):raise ValueError('Malformed compiled GTest discovery metadata')
    suites=data.get('testsuites')
    if type(data.get('tests')) is not int or not isinstance(suites,list) or not suites:
        raise ValueError('Missing compiled GTest source-origin metadata')
    for suite in suites:
        if not isinstance(suite,dict):raise ValueError('Malformed compiled GTest suite metadata')
        name=suite.get('name');cases=suite.get('testsuite')
        if not isinstance(name,str) or not name or not isinstance(cases,list) or type(suite.get('tests')) is not int or suite['tests']!=len(cases):
            raise ValueError('Malformed compiled GTest suite metadata')
        for case in cases:
            if not isinstance(case,dict):raise ValueError('Malformed compiled GTest case metadata')
            test=case.get('name');file=case.get('file');line=case.get('line')
            if not isinstance(test,str) or not test or not isinstance(file,str) or '\0' in file or type(line) is not int or line<=0:
                raise ValueError('Missing compiled GTest source origin')
            ident=name+'.'+test;path=Path(file);path=(path if path.is_absolute() else inventory.REPO_ROOT/path).resolve()
            if (ident in origins or not path.is_relative_to(inventory.REPO_ROOT/'unittest') or not path.is_file() or
                    path.suffix not in ('.c','.cc','.cpp','.cxx','.h','.hpp')):
                raise ValueError('Duplicate or foreign compiled GTest source origin')
            relative=path.relative_to(inventory.REPO_ROOT).as_posix()
            if relative not in inputs:raise ValueError('Unbound compiled GTest source: '+relative)
            if relative not in lines:
                content=path.read_bytes()
                if inputs[relative]!=sha256(content):raise ValueError('Unbound or changed compiled GTest source: '+relative)
                lines[relative]=len(content.splitlines())
            if line>lines[relative]:
                raise ValueError('Unbound or changed compiled GTest source: '+relative)
            origins[ident]=dict(path=relative,line=line)
            (widget if relative.startswith('unittest/ui/') else required).append(ident)
    if data['tests']!=len(origins) or set(origins)!=set(discovered) or not required:
        raise ValueError('Text/JSON GTest discovery differs or omits non-widget tests')
    return dict(required_ids=sorted(required),widget_ids=sorted(widget),origins=origins)


def installed_cli_files(build,prefix,original_prefix,names,windows=False):
    """Bind an owned install tree to CMake's manifest and actual target paths.

    This is a system-SDK install check, not a standalone runtime/package claim.
    The original manifest survives the subsequent prefix move unchanged.
    """
    prefix=prefix.resolve(strict=True);original_prefix=original_prefix.resolve()
    raw=(build/'install_manifest.txt').read_bytes();files={}
    for line in raw.decode().splitlines():
        path=Path(line)
        if not path.is_absolute() or not path.is_relative_to(original_prefix):
            raise ValueError('Foreign CLI install manifest path')
        relative=path.relative_to(original_prefix)
        target=prefix/relative
        if ('..' in relative.parts or (relative.as_posix() in files and not windows) or not target.is_file() or
                not target.resolve().is_relative_to(prefix)):
            raise ValueError('Missing, duplicate or escaping CLI install file')
        files[relative.as_posix()]=sha256(target.read_bytes())
    actual={p.relative_to(prefix).as_posix() for p in prefix.rglob('*') if p.is_file() or p.is_symlink()}
    if not files or set(files)!=actual:raise ValueError('CLI install tree differs from its manifest')
    targets=source_targets(build);installed={}
    for name in names:
        target=targets[name];destinations={d['path'] for d in target.get('install',{}).get('destinations',[])}
        artifacts=target.get('artifacts',[])
        if windows:
            # MSVC exposes the import archive and DLL in the same target. Only
            # the runtime output belongs beside the installed executable.
            suffix='.exe' if name=='luacmdinterface' else '.dll'
            artifacts=[a for a in artifacts if Path(a['path']).suffix.lower()==suffix]
            cache=(build/'CMakeCache.txt').read_text()
            bindir=re.findall(r'^CMAKE_INSTALL_BINDIR:[^=\n]+=([^\n]+)$',cache,re.M)
            if len(bindir)!=1 or bindir[0] not in destinations:raise ValueError('Missing Windows runtime install destination')
            destinations={bindir[0]}
        if len(destinations)!=1 or len(artifacts)!=1:
            raise ValueError('Missing or ambiguous CLI target installation: '+name)
        # File API reports both ARCHIVE and LIBRARY destinations for a shared
        # target even when they name the same directory on Unix.
        destination=Path(next(iter(destinations)))
        if destination.is_absolute() or '..' in destination.parts:
            raise ValueError('Non-relative CLI target installation: '+name)
        relative=(destination/Path(artifacts[0]['path']).name).as_posix()
        if relative not in files or relative in installed.values():
            raise ValueError('Missing or duplicate installed CLI target: '+name)
        installed[name]=relative
    return dict(manifest_sha256=sha256(raw),files_sha256=files,targets=installed)


def installed_cli_loads(raw,prefix,installed,macos,windows=False):
    """Require each project dependency to load from the relocated install tree."""
    text=raw.decode(errors='replace')
    paths=(text.splitlines() if windows else re.findall(r'^dyld\[\d+\]: (?:<[-0-9A-Fa-f]+> )?(/[^\r\n]+)$',text,re.MULTILINE) if macos else
           re.findall(r'calling init:\s+(/[^\r\n]+)',text))
    expected={Path(p).name:str((prefix/p).resolve()) for name,p in installed.items() if name!='luacmdinterface'}
    loaded={}
    for value in paths:
        path=Path(value.strip())
        if path.name not in expected:continue
        if path.resolve()!=Path(expected[path.name]) or not path.is_file() or not path.resolve().is_relative_to(prefix):
            raise ValueError('CLI loaded a project library outside its relocated prefix')
        loaded[path.name]=str(path.resolve())
    if set(loaded)!=set(expected):raise ValueError('Missing relocated CLI project-library load evidence')
    return dict(libraries=loaded,loader_log_sha256=sha256(raw))


def build_host_source(args):
    """Fresh real UI/CLI host build using installed SDKs, never a GUI-less alias."""
    if args.platform!=native_platform():raise ValueError('Host builds require the matching native platform')
    if args.app or args.build_dir or args.configuration or args.case_cmake_option or getattr(args,'_worker_directory',None):
        raise ValueError('Host checks require one fresh owned profile, not reused/graphics build inputs')
    if args.host_kind not in ('both','ui','cli') or (args.host_no_persistence and args.host_kind!='cli'):
        raise ValueError('Persistence OFF requires an explicit CLI-only host profile')
    install=getattr(args,'host_install',False)
    if install and args.host_kind!='cli':
        raise ValueError('CLI install checks require an explicit native CLI-only profile')
    ui=args.host_kind!='cli';cli=args.host_kind!='ui'
    sdk=build_sdk_options(args.cmake_option);helpers=helper_digests();inputs=production_inputs(hosts=True)
    tests=args.host_tests=='ON';python=args.host_profile.endswith('-python')
    flags=dict(WITH_QT_UI=ui,WITH_LUACMDINTERFACE=cli,WITH_PERSISTENCE=not args.host_no_persistence,
        WITH_CAIRO=args.host_cairo,WITH_LEGACY_OPENGL_RENDERER=True,WITH_RHI_RENDERER=args.host_profile.startswith('dual'),
        WITH_PYTHONSCRIPT=python,WITH_UNITTESTS=tests,WITH_RENDERER_TESTS=False,WITH_RENDERING_UNITTESTS=False,
        WITH_LIBOPENCAD=False,WITH_DOCUMENTATION=False,WITH_COVERAGE=False,TINYSPLINE_BUILD_EXAMPLES=False)
    args.report_dir.mkdir(parents=True,exist_ok=True)
    directory=Path(tempfile.mkdtemp(prefix='host-',dir=args.report_dir)).resolve();build=directory/'build'
    query=build/'.cmake/api/v1/query/client-renderer-source';query.mkdir(parents=True);(query/'codemodel-v2').touch()
    cmake=str(Path(args.cmake or shutil.which('cmake')).resolve(strict=True))
    deadline=time.monotonic()+1200;commands=[]
    env={k:v for k,v in os.environ.items() if not k.startswith('GTEST_') and k not in
         ('DISPLAY','XAUTHORITY','WAYLAND_DISPLAY','WAYLAND_SOCKET','PYTHONHOME','PYTHONPATH','DESTDIR','LC_RENDERER_DLL_REPORT')}
    env.update(QT_QPA_PLATFORM='offscreen',LIBRECAD_SETTINGS_DIR=str(directory/'settings'))
    def step(command,label,expected=0,environment=None):
        where=directory/label;where.mkdir();remaining=deadline-time.monotonic()
        if remaining<=0:raise ValueError('Host profile deadline expired; logs in '+str(directory))
        commands.append(run_command(command,where,min(args.timeout,remaining),env if environment is None else environment,expected_exit=expected))
        return where
    command=[cmake,'-S',str(inventory.REPO_ROOT),'-B',str(build),'-DCMAKE_BUILD_TYPE=Release',
        '-DCMAKE_CONFIGURATION_TYPES=Release','-DCMAKE_INSTALL_PREFIX='+str(directory/'stage'),
        '--trace-expand','--trace-format=json-v1','--trace-redirect='+str(directory/'trace.jsonl')]
    if args.generator:command+=['-G',args.generator]
    command+=['-D'+k+'='+v for k,v in sdk.items()]+['-D'+k+':BOOL='+('ON' if v else 'OFF') for k,v in flags.items()]
    forbidden=['Qt5','Qt5Core','Qt5Gui','Qt5Widgets']
    if not (ui or flags['WITH_RHI_RENDERER']):forbidden+=['Qt6']
    if not cli:forbidden+=['glfw','glfw3','CURL']
    if not flags['WITH_PERSISTENCE']:forbidden+=['libdxfrw']
    if not tests:forbidden+=['GTest','GMock','googletest']
    if not python:forbidden+=['Python','Python3','PythonInterp','PythonLibs']
    command+=['-DCMAKE_DISABLE_FIND_PACKAGE_'+p+'=ON' for p in forbidden]
    step(command,'configure');observed=library_profile_graph(build,flags,hosts=True);objects=host_object_graph(build,flags)
    if python and 'Python_EXECUTABLE' in sdk and Path(sdk['Python_EXECUTABLE']).resolve()!=Path(observed['python_provider']['executable']).resolve():
        raise ValueError('Host selected a different Python SDK')
    libraries=['lckernel','lcviewernoqt','lcviewer_opengl','lcscripting','lcluascript']
    if flags['WITH_PERSISTENCE']:libraries+=['persistence']
    if python:libraries+=['lcpythonscript']
    if flags['WITH_RHI_RENDERER']:libraries+=['lcrenderer_rhi']
    # Stage actual dependency targets explicitly; each command keeps its cap and
    # the entire profile shares the same 1,200-second deadline, including tests.
    host_names=(['librecad']+(['lcui'] if tests else []) if ui else [])+(['luacmdinterface'] if cli else [])
    # Each independent library retains the command cap; combined cold Python
    # and Lua compilation must not exhaust it before either target finishes.
    # All stages still share the original total profile deadline.
    for name in libraries:
        step([cmake,'--build',str(build),'--config','Release','--target',name,'--parallel','3'],'library-'+name)
    for label,names in (('ui-objects',['lcui_objects'] if ui else []),
            ('hosts',host_names),
            ('tests',['lcunittest','lcviewer_cpu_headercheck'] if tests else [])):
        if names:step([cmake,'--build',str(build),'--config','Release','--target',*names,'--parallel','3'],label)
    if install:
        # This C archive is installed by TinySpline but is not a CLI link
        # dependency. Build it before freezing binaries/testing; a later ALL
        # build would regenerate kernel build timestamps and relink consumers.
        step([cmake,'--build',str(build),'--config','Release','--target','tinyspline','--parallel','3'],
            'install-prerequisites')
    artifacts=library_artifacts(build,libraries+(['lcui_objects'] if ui else [])+host_names+(['lcunittest'] if tests else []),
        types=('STATIC_LIBRARY','SHARED_LIBRARY','OBJECT_LIBRARY','EXECUTABLE'))
    app=lambda name:build/'bin'/(name+('.exe' if sys.platform=='win32' else ''))
    if cli:
        where=step([str(app('luacmdinterface')),'--help'],'cli-help',1)
        if b'Allowed options' not in (where/'stdout.log').read_bytes():raise ValueError('Missing real CLI help')
    discovered=[];selection=dict(required_ids=[],widget_ids=[],origins={})
    if tests:
        where=step([str(app('lcunittest')),'--gtest_list_tests',
            '--gtest_output=json:'+str(directory/'discovery/compiled-tests.json')],'discovery')
        listing=re.sub(br'^LibreCAD v[^\r\n]+\r?\n',b'',(where/'stdout.log').read_bytes(),count=1)
        discovered=gtest_discovery(listing)
        selection=host_test_selection(inventory._bounded_bytes(where/'compiled-tests.json',8*1024*1024),discovered,inputs)
        if not ui and selection['widget_ids']:raise ValueError('CLI-only build contains widget test registrations')
        where=step([str(app('lcunittest')),'--gtest_filter='+':'.join(selection['required_ids']),
            '--gtest_output=xml:'+str(directory/'non-widget-tests/tests.xml')],'non-widget-tests')
        gtest_results((where/'tests.xml').read_bytes(),selection['required_ids'])
        cpu_header_contract(build,True)
    installation=None
    if install:
        stage=directory/'stage';relocated=directory/'relocated-prefix'
        step([cmake,'--install',str(build),'--config','Release','--prefix',str(stage)],'install')
        names=[n for n in libraries if n!='lcrenderer_rhi']+['luacmdinterface']
        windows=sys.platform=='win32'
        installation=installed_cli_files(build,stage,stage,names,windows)
        stage.rename(relocated)
        if installed_cli_files(build,relocated,stage,names,windows)!=installation:
            raise ValueError('CLI install changed during relocation')
        clean={k:v for k,v in env.items() if not k.startswith(('LD_','DYLD_'))}
        if windows:clean['LC_RENDERER_DLL_REPORT']=str(directory/'installed-cli-help/modules.txt')
        else:clean['DYLD_PRINT_LIBRARIES' if sys.platform=='darwin' else 'LD_DEBUG']='1' if sys.platform=='darwin' else 'libs'
        where=step([str(relocated/installation['targets']['luacmdinterface']),'--help'],
            'installed-cli-help',1,clean)
        if b'Allowed options' not in (where/'stdout.log').read_bytes():raise ValueError('Missing installed CLI help')
        raw=inventory._bounded_bytes(where/('modules.txt' if windows else 'stderr.log'),8*1024*1024)
        installation['loads']=installed_cli_loads(raw,relocated,installation['targets'],sys.platform=='darwin',windows)
        if installed_cli_files(build,relocated,stage,names,windows)!={k:v for k,v in installation.items() if k!='loads'}:
            raise ValueError('CLI install changed during execution')
    if (helpers!=helper_digests() or inputs!=production_inputs(hosts=True) or
            library_profile_graph(build,flags,hosts=True)!=observed or host_object_graph(build,flags)!=objects or
            library_artifacts(build,artifacts,types=('STATIC_LIBRARY','SHARED_LIBRARY','OBJECT_LIBRARY','EXECUTABLE'))!=artifacts):
        raise ValueError('Host inputs, graph or compiled artifacts changed during execution')
    record=dict(schema_version=1,evidence_kind='r1-host-source-build',phase_or_platform_accepted=False,
        platform=args.platform,profile=args.host_profile,host_kind=args.host_kind,options=flags,source_sha256=inputs,**helpers,
        source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=inventory.REPO_ROOT,text=True).strip(),
        commands=commands,observed=observed,ui_objects=objects,artifacts_sha256=artifacts,
        discovered_ids=discovered,executed_non_widget_ids=selection['required_ids'],test_origins=selection['origins'],
        unexecuted_widget_ids=selection['widget_ids'],runtime_graphics_executed=False,cli_help_executed=cli,
        installation=installation,installed_python_executed=False,
        profile_budget_seconds=1200,command_timeout_seconds=args.timeout)
    (directory/'host-build.json').write_text(json.dumps(record,indent=2)+'\n')
    print('R1 host source: PASS ('+args.host_kind+'/'+args.host_profile+', tests '+args.host_tests+'; not GUI/runtime acceptance); '+str(directory))
    return directory


def build_production_libraries(args):
    """Compile all eight GUI-less modes without discovering or running GTest."""
    if args.platform!=native_platform():raise ValueError('Production builds require the matching native platform')
    if args.app or args.build_dir or args.configuration or getattr(args,'_worker_directory',None):
        raise ValueError('Production builds require fresh owned trees, not a graphics/manifest test configuration')
    sdk=build_sdk_options(args.cmake_option);helpers=helper_digests();inputs=production_inputs()
    revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=inventory.REPO_ROOT,text=True).strip()
    args.report_dir.mkdir(parents=True,exist_ok=True)
    directory=Path(tempfile.mkdtemp(prefix='production-',dir=args.report_dir)).resolve()
    flags={k:False for k in ('WITH_QT_UI','WITH_LUACMDINTERFACE','WITH_PYTHONSCRIPT','WITH_CAIRO',
        'WITH_RHI_RENDERER','WITH_LEGACY_OPENGL_RENDERER','WITH_UNITTESTS','WITH_RENDERER_TESTS','WITH_RENDERING_UNITTESTS')}
    records=build_library_matrix(args,dict(configuration=dict(expected_options=flags)),sdk,directory,tests=False)
    for record in records:
        build=directory/'library-matrix'/record['profile']/'build'
        if (library_profile_graph(build,record['options'])!=record['observed'] or
                library_artifacts(build,record['artifacts_sha256'])!=record['artifacts_sha256']):
            raise ValueError('Production matrix inputs/artifacts changed')
    if helpers!=helper_digests() or inputs!=production_inputs():raise ValueError('Production source/helper changed during execution')
    record=dict(schema_version=1,evidence_kind='r1-production-build-matrix',phase_or_platform_accepted=False,
        runtime_tests_executed=False,platform=args.platform,source_revision=revision,source_sha256=inputs,
        **helpers,profiles=records)
    (directory/'production-build.json').write_text(json.dumps(record,indent=2)+'\n')
    print('R1 tests-OFF libraries: PASS (8 compile-only modes; no runtime acceptance); '+str(directory))
    return directory


def build_rhi_source(args):
    """Build the current GUI-less tuple using installed providers, never dependencies."""
    matrix=getattr(args,'library_matrix',False)
    if args.app or args.build_dir or (args.case_cmake_option and not matrix) or getattr(args,'_worker_directory',None):
        raise ValueError('Source build requires a fresh owned tree, not an app/build directory or case overrides')
    probe=argparse.Namespace(**vars(args));probe.app=Path('not-built');probe.build_dir=Path('not-built')
    selected=rhi_selection(probe);options=selected['configuration']['expected_options']
    if any(options[k] for k in ('WITH_QT_UI','WITH_LUACMDINTERFACE','WITH_PYTHONSCRIPT',
                                'WITH_LEGACY_OPENGL_RENDERER','WITH_CAIRO')):
        raise ValueError('Initial source build implements the GUI-less rhi-offscreen profile; host/dual/scripted gates remain required')
    sdk=build_sdk_options(args.cmake_option);helpers=helper_digests();header_inputs=cpu_header_inputs()
    cmake=Path(args.cmake or shutil.which('cmake') or '').resolve(strict=True)
    args.report_dir.mkdir(parents=True,exist_ok=True)
    directory=Path(tempfile.mkdtemp(prefix='source-',dir=args.report_dir)).resolve();build=directory/'build'
    libraries=build_library_matrix(args,selected,sdk,directory) if matrix else []
    query=build/'.cmake/api/v1/query/client-renderer-source';query.mkdir(parents=True);(query/'codemodel-v2').touch()
    env={k:v for k,v in os.environ.items() if not k.startswith('GTEST_') and k not in
         ('DISPLAY','XAUTHORITY','WAYLAND_DISPLAY','WAYLAND_SOCKET')}
    deadline=time.monotonic()+args.timeout;commands=[]
    def step(command,name):
        where=directory/name;where.mkdir()
        remaining=deadline-time.monotonic()
        if remaining<=0:raise ValueError('Source build deadline expired; logs in '+str(directory))
        commands.append(run_command(command,where,remaining,env))
    flags=dict(options,WITH_PERSISTENCE=False,WITH_LIBOPENCAD=False,WITH_DOCUMENTATION=False,
               WITH_COVERAGE=False,TINYSPLINE_BUILD_EXAMPLES=False)
    command=[str(cmake),'-S',str(inventory.REPO_ROOT),'-B',str(build),'-DCMAKE_BUILD_TYPE=Release',
             '-DCMAKE_CONFIGURATION_TYPES=Release','-DCMAKE_INSTALL_PREFIX='+str(directory/'stage')]
    if args.generator:command+=['-G',args.generator]
    command+=['-D'+k+'='+v for k,v in sdk.items()]
    command+=['-D'+k+':BOOL='+('ON' if v else 'OFF') for k,v in flags.items()]
    command+=['-DCMAKE_DISABLE_FIND_PACKAGE_'+p+'=ON' for p in ('Qt5','GLEW','glew','glfw3','glm','libdxfrw')]
    step(command,'configure')
    step([str(cmake),'--build',str(build),'--config','Release','--target',
          'lcrenderertest','lcrenderer_headercheck','lcviewer_cpu_headercheck','lcunittest','--parallel','3'],'compile')
    executable=build/'bin'/('lcunittest'+('.exe' if sys.platform=='win32' else ''))
    binary=sha256(executable.read_bytes());step([str(executable),'--gtest_list_tests'],'cpu-discovery')
    # Only remove lcunittest's established one-line version banner, not noise
    # or individual tests. Every discovered test must execute without a skip.
    listing=(directory/'cpu-discovery/stdout.log').read_bytes()
    listing=re.sub(br'^LibreCAD v[^\r\n]+\r?\n',b'',listing,count=1)
    discovered=gtest_discovery(listing);xml=directory/'cpu-tests/tests.xml'
    step([str(executable),'--gtest_output=xml:'+str(xml)],'cpu-tests')
    tests=gtest_results(xml.read_bytes(),discovered)
    if libraries and discovered!=libraries[0]['cpu_ids']:
        raise ValueError('RHI and library profiles compiled out different CPU IDs')
    if sha256(executable.read_bytes())!=binary or helper_digests()!=helpers or cpu_header_inputs()!=header_inputs:
        raise ValueError('Source CPU executable/helper changed during execution')
    artifacts={str(p.relative_to(directory)):sha256(p.read_bytes()) for p in directory.rglob('*')
        if p.is_file() and not p.is_symlink() and (p.suffix in ('.log','.json','.xml','.jsonl') or p.name=='CMakeCache.txt')}
    record=dict(schema_version=1,evidence_kind='r1-source-build',phase_or_platform_accepted=False,
        configuration=selected['configuration_id'],**helpers,commands=commands,options=flags,
        cpu_discovered_ids=discovered,cpu_executed_ids=sorted(tests),cpu_binary_sha256=binary,
        cmake=str(cmake),cmake_sha256=sha256(cmake.read_bytes()),artifacts_sha256=artifacts,library_matrix=libraries,
        cpu_headers=cpu_header_contract(build,True),cpu_header_sources_sha256=header_inputs)
    (directory/'source-build.json').write_text(json.dumps(record,indent=2)+'\n')
    args.build_dir=build;args.app=build/'bin'/('lcrenderertest'+('.exe' if sys.platform=='win32' else ''))
    return directory


def rhi_results(args, directory, selection):
    discovered = gtest_discovery(inventory._bounded_bytes(directory / 'discovery.txt',2*1024*1024))
    tests = gtest_results(inventory._bounded_bytes(directory / 'tests.xml',8*1024*1024), discovered)
    required = {r['test']:r for r in selection['requirements']}
    if not required.keys() <= tests.keys():
        raise ValueError('Required manifest GTest is missing')
    config = selection['configuration']
    prefix = args.provider_prefix.resolve(strict=True)
    shader_dir = args.build_dir.resolve(strict=True) / 'lcrenderer/rhi/.qsb'
    shader_hash = sha256(b''.join((shader_dir / ('triangles.' + n + '.qsb')).read_bytes() for n in ('vert','frag')))
    corpus = inventory.read_json(inventory.REPO_ROOT / selection['fixtures']['contracts']['path'])['datasets']
    observations, builds, modules = {}, [], []
    for name, props in tests.items():
        if 'rhi_case' in props and (name not in required or props['rhi_case'] != required[name]['id']):
            raise ValueError('Foreign/misbound RHI case observation')
        if name not in required:
            continue
        row = required[name]
        build = inventory._json_bytes(props.get('rhi_build', '').encode())
        fields = {'schema_version','evidence_kind','rhi_inputs','rhi_inputs_sha256','compiler','revision','git_dirty','qt','qsb',
                  'qsb_sha256','private_headers','options'}
        revision = subprocess.check_output(['git','rev-parse','HEAD'],cwd=inventory.REPO_ROOT,text=True).strip()
        dirty = bool(subprocess.check_output(['git','status','--porcelain','--untracked-files=normal'],cwd=inventory.REPO_ROOT))
        if (set(build) != fields or type(build['schema_version']) is not int or build['schema_version'] != 1 or
                build['evidence_kind'] != 'rhi-test-build-inputs' or
                json.dumps(build['options'],sort_keys=True) != json.dumps(config['expected_options'],sort_keys=True) or
                build['revision'] != revision or args.expected_revision not in (None,revision) or
                type(build['git_dirty']) is not bool or build['git_dirty'] != dirty or
                not isinstance(build['compiler'],str) or not build['compiler'].strip() or
                not re.fullmatch(r'6\.(?:[89]|[1-9][0-9])\.\d+', build['qt'])):
            raise ValueError('Missing/mismatched RHI build identity/options')
        inputs = build['rhi_inputs']
        entries = re.findall(r'^([0-9a-f]{64})  ([a-zA-Z0-9_./-]+)\n', inputs, re.M)
        paths = [p for _,p in entries]
        if (''.join(h+'  '+p+'\n' for h,p in entries) != inputs or paths != sorted(set(paths)) or
                not {'lcrenderer/rhi/renderer.cpp','lcrenderer/rhi/tests/buildtest.cpp',
                     'lcrenderer/rhi/tests/provenance.h','lcviewernoqt/painters/renderframe.cpp'} <= set(paths) or
                sha256(inputs.encode()) != build['rhi_inputs_sha256']):
            raise ValueError('Malformed/incomplete RHI input manifest')
        for digest, path in entries:
            p = (inventory.REPO_ROOT / path).resolve(strict=True)
            if '..' in Path(path).parts or not p.is_relative_to(inventory.REPO_ROOT) or sha256(p.read_bytes()) != digest:
                raise ValueError('RHI source differs from the compiled input: ' + path)
        qsb = Path(build['qsb']).resolve(strict=True)
        if not qsb.is_relative_to(prefix) or sha256(qsb.read_bytes()) != build['qsb_sha256']:
            raise ValueError('RHI shader baker differs from the build provider')
        headers = build['private_headers']
        if not isinstance(headers,dict) or len(headers) != 4 or {Path(p).name for p in headers} != {
                'qrhi.h','qrhi_platform.h','qshaderdescription.h','qshaderdescription_p.h'}:
            raise ValueError('Missing declared private Qt header identity')
        for path,digest in headers.items():
            p=Path(path).resolve(strict=True)
            if not p.is_relative_to(prefix) or sha256(p.read_bytes()) != digest:
                raise ValueError('Foreign/changed declared private Qt header')
        loaded = inventory._json_bytes(props.get('rhi_qt_modules', '').encode())
        if not isinstance(loaded, dict) or not 2 <= len(loaded) <= 64:
            raise ValueError('Missing/bounded loaded Qt module identity')
        for path,digest in loaded.items():
            p = Path(path).resolve(strict=True)
            if not p.is_relative_to(prefix) or Path(path) != p or sha256(p.read_bytes()) != digest:
                raise ValueError('Foreign/changed loaded Qt module')
        if not all(any(re.search(r'Qt6?'+part, p) for p in loaded) for part in ('Core','Gui')):
            raise ValueError('Loaded Qt Core/Gui modules missing')
        expected_arch = 'arm64' if config['platform'].endswith('ARM64') else 'x86_64'
        expected = {'rhi_api':config['backend'],'rhi_device_class':config['device'], 'rhi_qpa':config['qpa'],
                    'rhi_qt':build['qt'],'rhi_shader_sha256':shader_hash,
                    'rhi_fixture_sha256':selection['fixtures'][row['fixture']]['sha256'],
                    'rhi_arch':expected_arch,'rhi_build_arch':expected_arch,'rhi_case':row['id']}
        if any(props.get(k) != v for k,v in expected.items()) or not props.get('rhi_device','').strip():
            raise ValueError('RHI API/device/QPA/architecture/input identity mismatch')
        actual = inventory._json_bytes(props.get('rhi_observations', '').encode())
        assertions = corpus[row['dataset']][row['variant']]['assertions']
        if set(actual) != {a['observed'] for a in assertions}:
            raise ValueError('Missing/extra RHI observations')
        for assertion in assertions:
            target = assertion['expected']
            value = config[target['value']] if target['type'] == 'configuration' else target['value']
            if json.dumps(actual[assertion['observed']],sort_keys=True) != json.dumps(value,sort_keys=True):
                raise ValueError('RHI observation violates fixture: ' + assertion['observed'])
        if row['variant'] == 'ordinary' and props.get('rhi_indexed_draw_count') != '1':
            raise ValueError('RHI quad did not issue an indexed draw')
        if row['variant'] == 'limit' and (props.get('rhi_texture_limit') != '4' or int(props.get('rhi_texture_size_max','0')) <= 4):
            raise ValueError('RHI limit case must observe injected policy, not physical exhaustion')
        observations[row['id']] = actual
        builds.append(build); modules.append(loaded)
    if any(b != builds[0] for b in builds) or any(m != modules[0] for m in modules):
        raise ValueError('Inconsistent provider/build across required cases')
    return dict(configuration_id=selection['configuration_id'], configuration=config,
                manifest_sha256=selection['manifest_sha256'], required_ids=sorted(observations),
                discovered_ids=discovered, executed_ids=sorted(tests), observations=observations,
                build=builds[0], loaded_qt_sha256=modules[0], shader_sha256=shader_hash,
                observed_devices={row['id']:tests[row['test']]['rhi_device'] for row in selection['requirements']})


def rhi_icds(args):
    if args.device == 'software':
        forced = os.environ.get('VK_DRIVER_FILES') or os.environ.get('VK_ICD_FILENAMES','')
        if not args.icd_manifest or {Path(p).resolve(strict=True) for p in forced.split(os.pathsep) if p} != {p.resolve(strict=True) for p in args.icd_manifest}:
            raise ValueError('RHI software proof requires exactly declared forced ICD manifests')
    return [dict(path=str(p.resolve(strict=True)),sha256=sha256(p.read_bytes())) for p in args.icd_manifest]


def run_rhi(args):
    selection = rhi_selection(args)
    validation, helpers = validation_inputs(args), helper_digests()
    icds = rhi_icds(args)
    if sys.platform == 'linux' and (not validation or not args._worker_directory):
        raise ValueError('R1 Linux requires core/sync validation and the owned display supervisor')
    app = args.app.resolve(strict=True)
    binary_hash = sha256(app.read_bytes())
    args.report_dir.mkdir(parents=True, exist_ok=True)
    worker = args._worker_directory
    if worker:
        directory = worker.resolve(strict=True)
        if (worker.is_symlink() or directory.parent != args.report_dir.resolve() or not directory.name.startswith('run-') or
                {p.name for p in directory.iterdir()} != {'launcher.stdout.log','launcher.stderr.log'}):
            raise ValueError('RHI worker requires its fresh supervisor-owned directory')
        (directory / 'display.json').write_text(json.dumps(capture_display(os.environ),indent=2)+'\n')
    else:
        directory = Path(tempfile.mkdtemp(prefix='run-',dir=args.report_dir)).resolve()
    session = WestonSession(directory,args.weston,args.weston_prefix,args.weston_backend,None) if args.weston else None
    deadline = time.monotonic() + args.timeout
    with session if session else contextlib.nullcontext():
        env = {k:v for k,v in (session.client_env if session else os.environ).items() if not k.startswith('GTEST_')}
        env.update(LC_RENDERER_EXPECT_API=args.backend,LC_RENDERER_EXPECT_DEVICE=args.device,QT_QPA_PLATFORM=args.expected_qpa,
                   LC_RENDERER_CONTRACT_PATH=str(inventory.REPO_ROOT / selection['fixtures']['contracts']['path']))
        if validation:
            env = validation_environment(validation,directory,env)
        commands = [[str(app),'--gtest_list_tests'],[str(app),'--gtest_output=xml:'+str(directory/'tests.xml')]]
        for command,names in zip(commands,(('discovery.txt','discovery.stderr.log'),('stdout.log','stderr.log'))):
            try:
                result = subprocess.run(command,env=env,capture_output=True,timeout=max(0.1,deadline-time.monotonic()))
            except subprocess.TimeoutExpired as error:
                for name,data in zip(names,(error.stdout,error.stderr)):
                    (directory/name).write_bytes(data or b'')
                raise ValueError('RHI tests timed out; logs in '+str(directory)) from error
            for name,data in zip(names,(result.stdout,result.stderr)): (directory/name).write_bytes(data)
            if result.returncode:
                raise ValueError('RHI tests exited {}; logs in {}'.format(result.returncode,directory))
            if command == commands[0]:
                discovered = gtest_discovery(result.stdout)
                if not {r['test'] for r in selection['requirements']} <= set(discovered):
                    raise ValueError('Required manifest test compiled out')
    result = rhi_results(args,directory,selection)
    if (sha256(app.read_bytes()) != binary_hash or helper_digests() != helpers or
            rhi_selection(args) != selection or rhi_icds(args) != icds):
        raise ValueError('RHI executable/helpers/selection changed during execution')
    evidence = dict(schema_version=1,evidence_kind='r1-rhi-contract-runner',phase_or_platform_accepted=False,
                    app_sha256=binary_hash,commands=commands,**helpers,rhi=result,software_icds=icds,
                    vulkan_validation=validate_vulkan_validation(validation,directory),
                    host=dict(system=platform.system(),release=platform.release(),machine=platform.machine()))
    if session:
        evidence['wayland_compositor'] = session.record
    allowed = {'discovery.txt','discovery.stderr.log','tests.xml','stdout.log','stderr.log'}
    if validation: allowed.add('vk_layer_settings.txt')
    if worker: allowed.update(('display.json','launcher.stdout.log','launcher.stderr.log'))
    if session: allowed.update(session.files)
    if any(p.name not in allowed or not p.is_file() or p.is_symlink() for p in directory.iterdir()):
        raise ValueError('Unreported RHI output in the fresh evidence directory')
    evidence['artifacts_sha256'] = {p.name:sha256(p.read_bytes()) for p in directory.iterdir() if p.is_file() and
                                    p.name not in ('launcher.stdout.log','launcher.stderr.log')}
    (directory/('candidate.json' if worker else 'checks.json')).write_text(json.dumps(evidence,indent=2)+'\n')
    print('R1 RHI contracts: {} ({} required, {} executed); {}'.format(
        'candidate awaiting cleanup' if worker else 'PASS',len(selection['requirements']),len(result['executed_ids']),directory))
    return directory


def finish_rhi(args, directory, validation, helpers, record):
    validate_scope(record)
    candidate = directory / 'candidate.json'
    evidence = read_json(candidate)
    app = str(args.app.resolve(strict=True))
    commands = [[app,'--gtest_list_tests'],[app,'--gtest_output=xml:'+str(directory/'tests.xml')]]
    if (type(evidence.get('schema_version')) is not int or evidence['schema_version'] != 1 or
            evidence.get('evidence_kind') != 'r1-rhi-contract-runner' or evidence.get('phase_or_platform_accepted') is not False or
            evidence.get('host') != dict(system=platform.system(),release=platform.release(),machine=platform.machine()) or
            evidence.get('commands') != commands or
            helper_digests() != helpers or any(evidence.get(k) != v for k,v in helpers.items()) or
            sha256(args.app.resolve(strict=True).read_bytes()) != evidence.get('app_sha256') or
            rhi_results(args,directory,rhi_selection(args)) != evidence.get('rhi') or
            rhi_icds(args) != evidence.get('software_icds') or
            validation_inputs(args) != validation or
            validate_vulkan_validation(validation,directory) != evidence.get('vulkan_validation')):
        raise ValueError('RHI inputs/results changed during supervised cleanup')
    required = {'discovery.txt','discovery.stderr.log','tests.xml','stdout.log','stderr.log','display.json'}
    if validation: required.add('vk_layer_settings.txt')
    if not required <= evidence['artifacts_sha256'].keys():
        raise ValueError('Missing retained RHI artifacts')
    for name,digest in evidence['artifacts_sha256'].items():
        if Path(name).name != name or sha256((directory/name).read_bytes()) != digest:
            raise ValueError('RHI artifact changed during supervised cleanup')
    expected = set(evidence['artifacts_sha256']) | {'candidate.json','processes.json','launcher.stdout.log','launcher.stderr.log'}
    if {p.name for p in directory.iterdir()} != expected or any(not p.is_file() or p.is_symlink() for p in directory.iterdir()):
        raise ValueError('Unexpected RHI output after supervised cleanup')
    evidence['process_scope'] = record
    for name in ('processes.json','launcher.stdout.log','launcher.stderr.log'):
        evidence['artifacts_sha256'][name] = sha256((directory/name).read_bytes())
    candidate.write_text(json.dumps(evidence,indent=2)+'\n')
    candidate.replace(directory/'checks.json')
    print('R1 RHI contracts: PASS ({} required, {} executed; all owned descendants reaped); {}'.format(
        len(evidence['rhi']['required_ids']),len(evidence['rhi']['executed_ids']),directory))
    return directory


def run_isolated(args, argv):
    if sys.platform != 'linux' or args.backend != 'vulkan' or args.expected_qpa not in ('xcb', 'wayland'):
        raise ValueError('Isolated display requires Linux Vulkan and explicit XCB/Wayland QPA')
    if args.expected_qpa == 'wayland' and not args.weston:
        raise ValueError('Isolated Wayland requires a managed compositor')
    validation = validation_inputs(args)
    helpers = helper_digests()
    args.report_dir.mkdir(parents=True, exist_ok=True)
    directory = Path(tempfile.mkdtemp(prefix='run-', dir=args.report_dir)).resolve()
    launcher = args.xvfb_run.resolve(strict=True)
    tools = [launcher]
    for name in ('Xvfb', 'xauth'):
        tool = shutil.which(name)
        if not tool:
            raise ValueError('Missing isolated display tool: ' + name)
        tools.append(Path(tool))
    worker = [sys.executable, '-B', str(Path(__file__).resolve()), *without_launcher(argv),
              '--_worker-directory', str(directory)]
    env = os.environ.copy()
    for name in ('DISPLAY', 'XAUTHORITY', 'WAYLAND_DISPLAY', 'WAYLAND_SOCKET'):
        env.pop(name, None)
    env['QT_QPA_PLATFORM'] = args.expected_qpa
    record, runtime = {}, None
    try:
        with tempfile.TemporaryDirectory(prefix='lc-xvfb-', dir='/tmp') as runtime:
            auth = Path(runtime) / 'Xauthority'
            auth.touch(mode=0o600)
            command = [str(launcher), '-a', '--auth-file=' + str(auth),
                       '-s', '-screen 0 2304x1792x24 -nolisten tcp', *worker]
            record = supervise(command, directory, args.timeout + 20, env, tools)
    except BaseException as error:
        record.setdefault('errors', []).append(type(error).__name__ + ': ' + str(error))
    finally:
        record['runtime_directory_removed'] = runtime is not None and not Path(runtime).exists()
        record['display_files_retired'] = False
        if (directory / 'display.json').is_file():
            try:
                record['display'] = read_json(directory / 'display.json')
                validate_display_retired(record['display'], record)
                record['display_files_retired'] = True
            except (OSError, ValueError, TypeError, KeyError) as error:
                record.setdefault('errors', []).append('display cleanup: ' + str(error))
        (directory / 'processes.json').write_text(json.dumps(record, indent=2) + '\n')
    try:
        validate_scope(record)
    except ValueError as error:
        log = directory / 'launcher.stderr.log'
        detail = ''
        if log.is_file():
            with log.open('rb') as stream:
                stream.seek(max(0, log.stat().st_size - 2048))
                detail = stream.read().decode(errors='replace')
        raise ValueError('{}; launcher exit {}; logs in {}; {}'.format(
            error, record.get('launcher_exit_status'), directory, detail)) from error
    if getattr(args,'rhi_contracts',False):
        return finish_rhi(args,directory,validation,helpers,record)
    candidate = directory / 'candidate.json'
    evidence = read_json(candidate)
    if helper_digests() != helpers or any(evidence.get(key) != value for key, value in helpers.items()):
        raise ValueError('Renderer helper source changed during supervised execution')
    if sha256(args.app.resolve(strict=True).read_bytes()) != evidence['app_sha256']:
        raise ValueError('Renderer executable changed during supervised execution')
    expected_failure = args.expect_failure
    report = directory / ('failure.json' if expected_failure else 'app.json')
    if sha256(report.read_bytes()) != evidence['app_report_sha256'] or read_json(report) != evidence['app_report']:
        raise ValueError('Renderer report changed during supervised cleanup')
    if expected_failure:
        checks = validate_failure(report, args.backend, args.device, expected_failure, args.expected_revision,
                                  args.expected_arch or platform.machine(), args.expected_dpr, args.expected_qpa, args.background)
    else:
        checks = validate_report(report, args.backend, args.device, args.mode, args.expected_revision,
                                 args.expected_arch or platform.machine(), args.expected_dpr, args.expected_qpa,
                                 args.background, args.scenario)
    if checks != evidence['checks']['expected_failure_ids' if expected_failure else 'executed_smoke_ids']:
        raise ValueError('Renderer checks changed during supervised cleanup')
    if validation_inputs(args) != validation or validate_vulkan_validation(validation, directory) != evidence.get('vulkan_validation'):
        raise ValueError('Vulkan validation evidence changed during supervised cleanup')
    allowed = {report.name, 'candidate.json', 'display.json', 'stdout.log', 'stderr.log', 'launcher.stdout.log',
               'launcher.stderr.log', 'processes.json'}
    for output in evidence['app_report'].get('outputs', []):
        allowed.update((output['image'], output['rgba']))
    if validation:
        allowed.add('vk_layer_settings.txt')
    if args.weston:
        allowed.update(WestonSession.files)
        for name, digest in evidence['wayland_compositor_artifacts'].items():
            if name not in WestonSession.files | {'wayland-input.json'} or sha256((directory / name).read_bytes()) != digest:
                raise ValueError('Compositor artifact changed during supervised cleanup')
            allowed.add(name)
    if any(path.name not in allowed or not path.is_file() for path in directory.iterdir()):
        raise ValueError('Unreported output after supervised cleanup')
    evidence['process_scope'] = record
    evidence['process_scope_sha256'] = sha256((directory / 'processes.json').read_bytes())
    evidence['process_artifacts_sha256'] = {name: sha256((directory / name).read_bytes())
                                           for name in ('processes.json', 'display.json', 'launcher.stdout.log', 'launcher.stderr.log')}
    candidate.write_text(json.dumps(evidence, indent=2) + '\n')
    candidate.replace(directory / 'checks.json')
    print('R0 renderer {}: PASS ({} checks; all owned descendants reaped); {}'.format(
        'expected failure' if expected_failure else 'checks', len(checks), directory))
    return directory


def run(args):
    weston = getattr(args, "weston", None)
    if weston and (args.backend != "vulkan" or args.expected_qpa != "wayland"):
        raise ValueError("Weston proof requires forced Vulkan and --expected-qpa wayland")
    if weston and args.mode == "widget" and args.scenario == "lifecycle" and not args.weston_input_tool:
        raise ValueError("Wayland lifecycle requires a qualified compositor input seat/tool")
    expected_failure = getattr(args, "expect_failure", None)
    if expected_failure and (expected_failure not in ("render", "readback", "shutdown") or
                             args.mode != "widget" or args.scenario != "standard"):
        raise ValueError("expected failure requires standard widget render, readback or shutdown")
    if args.scenario == "recovery" and args.mode != "offscreen":
        raise ValueError("recovery requires the owned offscreen host")
    if args.scenario == "reparent" and args.mode != "widget":
        raise ValueError("reparent requires the borrowed widget host")
    validation = validation_inputs(args)
    helpers = helper_digests()
    app = args.app.resolve(strict=True)
    args.report_dir.mkdir(parents=True, exist_ok=True)
    worker = getattr(args, '_worker_directory', None)
    if worker:
        directory = worker.resolve(strict=True)
        if worker.is_symlink() or directory.parent != args.report_dir.resolve() or not directory.name.startswith('run-') or {p.name for p in directory.iterdir()} != {'launcher.stdout.log', 'launcher.stderr.log'}:
            raise ValueError('Worker requires the fresh supervisor-owned evidence directory')
        (directory / 'display.json').write_text(json.dumps(capture_display(os.environ), indent=2) + '\n')
    else:
        directory = Path(tempfile.mkdtemp(prefix="run-", dir=args.report_dir))
    report = directory / "app.json"
    binary_digest = sha256(app.read_bytes())
    command = [str(app), "--backend=" + args.backend, "--renderer-report=" + str(report.resolve()),
               "--render-output=" + str((directory / "frame.png").resolve()),
               "--background=" + args.background, "--scenario=" + args.scenario]
    if expected_failure:
        command.append("--inject-failure=" + expected_failure)
    session = WestonSession(directory, weston, args.weston_prefix, args.weston_backend,
                            args.weston_input_tool if args.mode == "widget" and args.scenario == "lifecycle" else None) if weston else None
    try:
        with session if session else contextlib.nullcontext():
            env = None
            if validation:
                env = validation_environment(validation, directory, session.client_env if session else os.environ)
                if session:
                    session.client_env = env
            result = session.execute(command, args.timeout) if session else subprocess.run(command, capture_output=True, timeout=args.timeout, **({'env': env} if env else {}))
            (directory / "stdout.log").write_bytes(result.stdout)
            (directory / "stderr.log").write_bytes(result.stderr)
            validation_result = validate_vulkan_validation(validation, directory)
            if result.returncode != (1 if expected_failure else 0):
                raise ValueError("renderer exited {}: {}; logs in {}".format(
                    result.returncode, result.stderr.decode(errors="replace")[-2000:], directory))
    except subprocess.TimeoutExpired as error:
        for name, output in (("stdout.log", error.stdout), ("stderr.log", error.stderr)):
            if output is not None or not (directory / name).exists():
                (directory / name).write_bytes(output or b"")
        raise ValueError("renderer timed out; logs in " + str(directory)) from error
    if sha256(app.read_bytes()) != binary_digest:
        raise ValueError("executable changed during execution")
    if expected_failure:
        report = directory / "failure.json"
        checks = validate_failure(report, args.backend, args.device, expected_failure, args.expected_revision,
                                  args.expected_arch or platform.machine(), args.expected_dpr, args.expected_qpa,
                                  args.background)
    else:
        checks = validate_report(report, args.backend, args.device, args.mode, args.expected_revision,
                                 args.expected_arch or platform.machine(), args.expected_dpr, args.expected_qpa,
                                 args.background, args.scenario)
    app_report = read_json(report)
    allowed = {report.name, "stdout.log", "stderr.log"}
    if validation:
        allowed.add('vk_layer_settings.txt')
    if worker:
        allowed.update(('display.json', 'launcher.stdout.log', 'launcher.stderr.log'))
    native_input = None
    if args.mode == "widget" and args.scenario == "lifecycle" and app_report.get("runtime", {}).get("qpa") == "wayland":
        native_input = directory / "wayland-input.json"
        validate_input_marker(native_input)
        allowed.add(native_input.name)
    if session:
        allowed.update(session.files)
    for output in app_report.get("outputs", []):
        allowed.update((output["image"], output["rgba"]))
    if any(path.name not in allowed or not path.is_file() for path in directory.iterdir()):
        raise ValueError("unreported output in fresh evidence directory (including possible failed-frame pixels)")
    icds = []
    if args.device == "software":
        selection = os.environ.get("VK_DRIVER_FILES") or os.environ.get("VK_ICD_FILENAMES", "")
        selected = {Path(value).resolve() for value in selection.split(os.pathsep) if value}
        declared = {path.resolve() for path in args.icd_manifest}
        if not declared or selected != declared:
            raise ValueError("software qualification requires exactly the declared ICD manifests to be forced in the loader environment")
    for path in args.icd_manifest:
        icds.append({"manifest": path.name, "sha256": sha256(path.read_bytes())})
    if helper_digests() != helpers:
        raise ValueError("renderer helper source changed during execution")
    evidence = {"schema_version": 3, "evidence_kind": "r0-spike-expected-failure" if expected_failure else "r0-spike-runner",
                "app_sha256": binary_digest,
                "app_report_sha256": sha256(report.read_bytes()), "command": command,
                **helpers,
                "checks": {"executed_smoke_ids": [] if expected_failure else checks, "failed": 0, "skipped": 0,
                           "gtest": None, "scope": "R0 landmarks; not production renderer suites"},
                "software_icds": icds, "app_report": app_report}
    if session:
        evidence["wayland_compositor"] = session.record
        evidence["wayland_compositor_artifacts"] = {name: sha256((directory / name).read_bytes()) for name in sorted(session.files)}
    if native_input:
        evidence["wayland_input_sha256"] = sha256(native_input.read_bytes())
    if validation:
        evidence['vulkan_validation'] = validation_result
    if expected_failure:
        evidence["checks"].update(expected_failure_ids=checks, scope="R0 expected terminal failure; no rendered-success claim")
    (directory / ('candidate.json' if worker else 'checks.json')).write_text(json.dumps(evidence, indent=2) + "\n", encoding="utf-8")
    print("R0 renderer {}: {} ({} checks); {}".format("expected failure" if expected_failure else "checks",
          'candidate awaiting process cleanup' if worker else 'PASS', len(checks), directory))
    return directory


def main():
    parser = argparse.ArgumentParser(description=__doc__, allow_abbrev=False)
    scope = parser.add_mutually_exclusive_group(required=True)
    scope.add_argument("--spike", action="store_true",
                        help="Explicit R0 scope; does not certify the application migration")
    scope.add_argument('--rhi-contracts',action='store_true',help='Provisional R1 source/offscreen GTests, not full R1 acceptance')
    scope.add_argument('--build-contracts',action='store_true',help='Provisional R1 CPU CMake cases plus native GTest inspection')
    scope.add_argument('--production-builds',action='store_true',help='Provisional R1 tests-OFF GUI-less library compilation only')
    scope.add_argument('--host-builds',action='store_true',help='Provisional R1 fresh real UI/CLI source checks; no widget/GPU acceptance')
    parser.add_argument('--host-profile',choices=HOST_PROFILES,help='Required explicit host renderer/Python profile')
    parser.add_argument('--host-kind',choices=('both','ui','cli'),help='Real host selection (default both); never an SDK fallback')
    parser.add_argument('--host-no-persistence',action='store_true',help='Explicit persistence OFF; requires --host-kind cli')
    parser.add_argument('--host-install',action='store_true',help='Install, move and loader-audit a native CLI-only profile using its selected SDKs')
    parser.add_argument('--host-tests',choices=('ON','OFF'),help='Host unit-test selection (default ON); OFF forbids GTest discovery')
    parser.add_argument('--host-cairo',action='store_true',help='Also require the installed Cairo provider and its host test')
    parser.add_argument('--platform',choices=sorted(inventory.CASE_PLATFORMS),help='Exact native platform for production/host builds')
    parser.add_argument('--build-source',action='store_true',help='With --rhi-contracts, build fresh GUI-less source and run all CPU tests first')
    parser.add_argument('--library-matrix',action='store_true',help='With --build-source, also build/test fresh CPU, legacy and dual libraries; not host/pixel acceptance')
    parser.add_argument('--configuration',help='Exact manifest configuration ID for R1; no hand-picked case subsets')
    parser.add_argument('--build-dir',type=Path,help='R1 CMake build containing the tested offline shader packages')
    parser.add_argument('--provider-prefix',type=Path,help='Required actual Qt/qsb provider boundary for R1')
    parser.add_argument("--app", type=Path)
    parser.add_argument("--backend", choices=("vulkan", "metal"))
    parser.add_argument("--device", choices=("software", "hardware", "any"))
    parser.add_argument("--mode", choices=("offscreen", "widget"))
    parser.add_argument('--cmake',type=Path,help='Build-contract CMake executable; defaults to PATH')
    parser.add_argument('--generator',help='Build-contract CMake generator, including multi-config generators')
    parser.add_argument('--trace-format',choices=('auto','json-v1','human'),default='auto',help='Build trace adapter; auto preserves the CPU CMake 3.15 floor')
    parser.add_argument('--cmake-option',action='append',default=[],help='Non-fixture SDK KEY=VALUE, applied to fresh build cases')
    parser.add_argument('--case-cmake-option',action='append',default=[],help='VARIANT:KEY=VALUE SDK override, e.g. matched per-case toolchains')
    parser.add_argument("--report-dir", type=Path, required=True)
    parser.add_argument("--timeout", type=float, default=30)
    parser.add_argument("--expected-revision")
    parser.add_argument("--expected-arch")
    parser.add_argument("--expected-dpr", type=float)
    parser.add_argument("--expected-qpa")
    parser.add_argument("--background", choices=("opaque", "transparent"), default="opaque")
    parser.add_argument("--scenario", choices=("standard", "lifecycle", "recovery", "reparent"), default="standard")
    parser.add_argument("--expect-failure", choices=("render", "readback", "shutdown"),
                        help="Require exact standard-widget terminal failure, not rendered-success evidence")
    parser.add_argument("--icd-manifest", type=Path, action="append", default=[])
    parser.add_argument('--vulkan-validation-manifest', type=Path, help='Require Linux core/synchronization validation and actual loaded-layer proof')
    parser.add_argument('--vulkan-validation-library', type=Path, help='Exact prebuilt Khronos validation .so paired with the manifest')
    parser.add_argument("--weston", type=Path, help="Isolated qualified Weston 14 session; not an app backend fallback")
    parser.add_argument("--weston-prefix", type=Path, help="Extracted/system provider prefix (default: executable prefix)")
    parser.add_argument("--weston-backend", choices=("headless", "x11"), default="headless")
    parser.add_argument("--weston-input-tool", type=Path, help="Qualified xdotool F12; lifecycle only, targeted to Weston on Xvfb")
    parser.add_argument('--xvfb-run', type=Path, help='Own an isolated Xvfb process tree; accept results only after reaping every descendant')
    parser.add_argument('--_worker-directory', type=Path, help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.build_source and not args.rhi_contracts:parser.error('--build-source requires --rhi-contracts')
    if args.library_matrix and not args.build_source:parser.error('--library-matrix requires --build-source')
    if args.host_builds:
        if not args.host_profile:parser.error('--host-builds requires --host-profile')
        if args.host_kind is None:args.host_kind='both'
        if args.host_no_persistence and args.host_kind!='cli':parser.error('--host-no-persistence requires --host-kind cli')
        if args.host_install and args.host_kind!='cli':
            parser.error('--host-install requires a native --host-kind cli profile')
        if args.host_tests is None:args.host_tests='ON'
    elif args.host_profile or args.host_tests or args.host_cairo or args.host_kind or args.host_no_persistence or args.host_install:
        parser.error('Host profile/test/Cairo arguments require --host-builds')
    if args.production_builds or args.host_builds:
        if not args.platform:parser.error('Production/host builds require --platform')
        if any(getattr(args,k) for k in ('backend','device','mode','provider_prefix','expected_qpa','xvfb_run',
                'weston','weston_prefix','weston_input_tool','vulkan_validation_manifest','vulkan_validation_library',
                'icd_manifest','expected_revision','expected_arch','expected_dpr','expect_failure')) or (
                args.trace_format!='auto' or args.scenario!='standard' or args.background!='opaque'):
            parser.error('Production compilation cannot substitute for graphics/runtime checks')
    elif args.platform:parser.error('--platform requires --production-builds or --host-builds')
    if not (args.build_contracts or args.production_builds or args.host_builds):
        for name in ('app','backend','device','mode'):
            if name=='app' and args.build_source:continue
            if getattr(args,name) is None:parser.error('--'+name+' is required for graphics checks')
        if (((args.cmake or args.generator or args.cmake_option) and not args.build_source) or
                (args.case_cmake_option and not args.library_matrix) or args.trace_format!='auto'):
            parser.error('CMake build arguments require --build-contracts')
    if not 0 < args.timeout <= 300:
        parser.error("timeout must be in (0, 300]")
    if args.xvfb_run and args._worker_directory:
        parser.error('A supervised worker cannot recursively launch another display')
    try:
        if args.host_builds:
            build_host_source(args)
            return
        if args.production_builds:
            build_production_libraries(args)
            return
        if args.build_contracts:
            run_build_contracts(args)
            return
        argv=sys.argv[1:]
        if args.build_source:
            build_rhi_source(args)
            # The Linux worker executes this binary, never a recursive build.
            argv=source_worker_argv(argv,args)
        if args.rhi_contracts:
            rhi_selection(args) # Fail tuple/scope mistakes before launching a display or device.
        run_isolated(args, argv) if args.xvfb_run else run_rhi(args) if args.rhi_contracts else run(args)
    except (OSError, ValueError, TypeError, KeyError, subprocess.SubprocessError, zlib.error) as error:
        parser.exit(1, "{} renderer checks: FAIL: {}\n".format('R1' if args.rhi_contracts or args.build_contracts or args.production_builds or args.host_builds else 'R0',error))


if __name__ == "__main__":
    main()
