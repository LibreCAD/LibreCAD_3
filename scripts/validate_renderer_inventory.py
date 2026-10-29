#!/usr/bin/env python3
"""Validate R0 pixels or logical renderer requirements; not release/package authority."""

import argparse
import hashlib
import itertools
import json
import math
import re
import struct
import zlib
from pathlib import Path

SPIKE_ROOT = Path(__file__).resolve().parents[1] / "lcrenderer/rhi/spike"
FIXTURE_PATH = SPIKE_ROOT / "fixture.json"
REPO_ROOT = SPIKE_ROOT.parents[2]
CASE_PATH = REPO_ROOT / 'unittest/rendering/renderer_cases.json'

# Required source tuples from the migration contract, not runner availability.
CASE_PLATFORMS = {
    'LINUX-X64': ('vulkan', ('xcb', 'wayland'), ('software', 'hardware')),
    'LINUX-ARM64': ('vulkan', ('xcb', 'wayland'), ('software', 'hardware')),
    'WIN-X64': ('vulkan', ('windows',), ('software', 'hardware')),
    'MAC-ARM64': ('metal', ('cocoa',), ('hardware',)),
    'MAC-X64': ('metal', ('cocoa',), ('hardware',)),
}
# Reviewed build intents, not observed CMake flags or accepted provider routes.
_FEATURE_OPTIONS = {'rhi': 'WITH_RHI_RENDERER', 'legacy': 'WITH_LEGACY_OPENGL_RENDERER',
                    'ui': 'WITH_QT_UI', 'cli': 'WITH_LUACMDINTERFACE',
                    'python': 'WITH_PYTHONSCRIPT', 'cairo': 'WITH_CAIRO'}
# RHI, legacy, Widgets, CLI, Python, Cairo. Lua remains an existing core dependency.
_PROFILE_FLAGS = {'cpu': (0, 0, 0, 0, 0, 0), 'cairo': (0, 0, 0, 0, 0, 1),
                  'rhi-only': (1, 0, 1, 0, 0, 0), 'dual': (1, 1, 1, 0, 0, 0),
                  'scripted': (1, 0, 1, 0, 1, 0), 'rhi-offscreen': (1, 0, 0, 0, 0, 0),
                  'rhi-cli': (1, 0, 0, 1, 0, 0), 'rhi-cli-python': (1, 0, 0, 1, 1, 0),
                  'dual-legacy': (1, 1, 1, 0, 0, 0)}
PROFILE_MODES = {name: ({'cpu'} if name in ('cpu', 'cairo') else {'widget'} if name == 'dual-legacy'
                      else {'offscreen', 'widget'} if flags[2] else {'offscreen'})
                 for name, flags in _PROFILE_FLAGS.items()}
ORACLE_UNITS = {'position-error': 'model', 'area-error': 'model-squared',
                'angle-error': 'radian', 'scalar-error': 'ratio',
                'curve-error': 'physical-pixel', 'channel-error': '8bit-channel',
                'font-metric-error': 'font-unit', 'contract-violations': 'count',
                'memory-growth': 'ratio', 'interaction-latency': 'ratio', 'interaction-time': 'second',
                'cold-open-latency': 'ratio', 'idle-submissions': 'count'}

_NUMBER_UNITS = {'model', 'model-squared', 'physical-pixel', 'logical-pixel',
                 'radian', 'ratio', 'font-unit', 'second', 'byte', 'count'}
_INTEGER_UNITS = {'count', 'byte', 'codepoint', 'font-unit', 'physical-pixel', 'logical-pixel'}
_VALUE_UNITS = {'number': _NUMBER_UNITS, 'numbers': _NUMBER_UNITS,
                'integer': _INTEGER_UNITS, 'integers': _INTEGER_UNITS,
                'vec2': {'model', 'physical-pixel', 'logical-pixel', 'font-unit'},
                'points2': {'model', 'physical-pixel', 'logical-pixel', 'font-unit'},
                'bytes': {'byte', '8bit-channel'}, 'matrix3': {'none'},
                'boolean': {'none'}, 'text': {'none'}, 'strings': {'none'}, 'asset': {'none'},
                'configuration': {'none'}, 'nonfinite': _NUMBER_UNITS}


def _identifier(value, label):
    require(isinstance(value, str) and re.fullmatch(r'[a-z][a-z0-9-]{0,79}', value), 'invalid ' + label)
    return value


def _references(values, allowed, label, optional=False):
    require(isinstance(values, list) and len(values) <= 256 and (optional or values), 'invalid ' + label)
    result = _unique(values, label) if values else set()
    require(result <= allowed, 'unknown ' + label)
    return result


def _typed_value(record, expected=False):
    """Bounded data values for future native tests, not a scene interpreter."""
    _fields(record, ('type', 'unit', 'value'))
    kind, unit = _string(record, 'type'), _string(record, 'unit')
    require(kind in _VALUE_UNITS and unit in _VALUE_UNITS[kind], 'incompatible input/assertion type/unit')
    value = record['value']

    def number(v):
        return type(v) in (int, float) and abs(v) <= 1e15 and math.isfinite(v)

    def integer(v):
        return (type(v) is int and abs(v) <= 2 ** 53 - 1 and
                (unit != 'codepoint' or 0 <= v <= 0x10ffff and not 0xd800 <= v <= 0xdfff))

    def string(v):
        return isinstance(v, str) and '\0' not in v and len(v.encode('utf-8')) <= 4096

    def array(v, predicate, maximum=512):
        return isinstance(v, list) and len(v) <= maximum and all(predicate(x) for x in v)

    valid = {
        'number': lambda: number(value), 'integer': lambda: integer(value),
        'boolean': lambda: type(value) is bool, 'text': lambda: string(value),
        'numbers': lambda: array(value, number), 'integers': lambda: array(value, integer),
        'strings': lambda: array(value, string, 128),
        'vec2': lambda: array(value, number, 2) and len(value) == 2,
        'points2': lambda: array(value, lambda p: array(p, number, 2) and len(p) == 2),
        'matrix3': lambda: array(value, number, 9) and len(value) == 9 and value[6:] == [0, 0, 1],
        'bytes': lambda: array(value, lambda v: integer(v) and 0 <= v <= 255, 4096),
        'configuration': lambda: expected and isinstance(value, str) and
            value in ('backend', 'platform', 'mode', 'qpa', 'device', 'profile', 'artifact'),
        'asset': lambda: not expected and isinstance(value, str) and bool(re.fullmatch(r'[a-z][a-z0-9-]{0,79}', value)),
        'nonfinite': lambda: not expected and isinstance(value, str) and value in ('NaN', 'Infinity', '-Infinity'),
    }[kind]()
    require(valid, 'malformed/bounded input or assertion value: ' + kind)


def _case_oracles(values):
    require(isinstance(values, list) and 1 <= len(values) <= 32, 'invalid oracle inventory')
    oracles = {}
    for oracle in values:
        _fields(oracle, ('id', 'metric', 'unit', 'limit', 'decision', 'reference'))
        ident, metric = _identifier(oracle['id'], 'oracle ID'), _string(oracle, 'metric')
        require(ident not in oracles and metric in ORACLE_UNITS and oracle['unit'] == ORACLE_UNITS[metric],
                'duplicate oracle or incompatible metric/unit')
        limit = oracle['limit']
        require(type(limit) in (int, float) and 0 <= limit <= 1e15 and math.isfinite(limit) and
                isinstance(oracle['decision'], str) and oracle['decision'] in {'D' + str(i) for i in range(1, 7)} and
                _string(oracle, 'reference'), 'invalid/unowned oracle threshold')
        if metric == 'contract-violations':
            require(limit == 0, 'categorical contracts cannot tolerate violations')
        oracles[ident] = oracle
    return oracles


def validate_contract_inputs(payload, oracles):
    _fields(payload, ('schema_version', 'evidence_kind', 'datasets'))
    require(type(payload['schema_version']) is int and payload['schema_version'] == 1 and
            payload['evidence_kind'] == 'renderer-contract-inputs', 'wrong contract-input schema/scope')
    datasets = payload['datasets']
    require(isinstance(datasets, dict) and 1 <= len(datasets) <= 128, 'invalid contract dataset inventory')
    for ident, variants in datasets.items():
        _identifier(ident, 'dataset ID')
        require(isinstance(variants, dict) and 3 <= len(variants) <= 16 and
                {'ordinary', 'empty', 'limit'} <= variants.keys(), 'missing/bounded dataset variants')
        for name, variant in variants.items():
            _identifier(name, 'dataset variant')
            _fields(variant, ('inputs', 'assertions'))
            inputs = variant['inputs']
            require(isinstance(inputs, dict) and 1 <= len(inputs) <= 64, 'absent/bounded typed inputs')
            for key, value in inputs.items():
                _identifier(key, 'input field')
                _typed_value(value)
            assertions = variant['assertions']
            require(isinstance(assertions, list) and 1 <= len(assertions) <= 32, 'absent/bounded assertions')
            observed = set()
            for assertion in assertions:
                _fields(assertion, ('observed', 'expected', 'oracle'))
                field = _identifier(assertion['observed'], 'observed field')
                require(field not in observed, 'duplicate observed assertion')
                observed.add(field)
                _typed_value(assertion['expected'], expected=True)
                reference = _string(assertion, 'oracle')
                require(reference in oracles, 'unknown assertion oracle')
                oracle, expected = oracles[reference], assertion['expected']
                if oracle['metric'] == 'contract-violations':
                    require((expected['unit'] == 'none' and expected['type'] in
                             ('boolean', 'text', 'strings', 'configuration')) or
                            (expected['unit'] in ('count', 'codepoint') and expected['type'] in ('integer', 'integers')),
                            'noncategorical assertion requires a dimensional oracle')
                else:
                    require(oracle['metric'] in ('position-error', 'area-error', 'angle-error', 'scalar-error', 'curve-error', 'channel-error',
                                                'font-metric-error', 'idle-submissions') and
                            expected['unit'] == oracle['unit'] and expected['type'] in
                            ('number', 'integer', 'numbers', 'integers', 'vec2', 'points2', 'bytes'),
                            'assertion/oracle metric or unit mismatch')
    return datasets


def _case_fixtures(values, root, oracles):
    require(isinstance(values, list) and 1 <= len(values) <= 32, 'invalid fixture inventory')
    fixtures, datasets, paths = {}, {}, set()
    for fixture in values:
        _fields(fixture, ('id', 'path', 'sha256', 'kind', 'license', 'notice', 'notice_sha256', 'provenance'))
        ident = _identifier(fixture['id'], 'fixture ID')
        require(ident not in fixtures and isinstance(fixture['kind'], str) and
                fixture['kind'] in ('contracts', 'font', 'cad-drawing', 'script-source'), 'duplicate/unknown fixture')
        data_path, notice_path = _repo_file(fixture['path'], root), _repo_file(fixture['notice'], root)
        require(data_path not in paths, 'duplicate fixture path')
        paths.add(data_path)
        data, notice = _bounded_bytes(data_path, 8 * 1024 * 1024), _bounded_bytes(notice_path, 128 * 1024)
        require(_digest(fixture, 'sha256') == sha256(data) and
                _digest(fixture, 'notice_sha256') == sha256(notice), 'stale fixture/license identity')
        require(isinstance(fixture['license'], str) and fixture['license'] in ('GPL-3.0-or-later', 'OFL-1.1') and
                _string(fixture, 'provenance'), 'missing fixture provenance')
        if fixture['kind'] == 'font':
            require(fixture['license'] == 'OFL-1.1' and b'SIL OPEN FONT LICENSE Version 1.1' in notice and
                    b'PERMISSION & CONDITIONS' in notice and b'DISCLAIMER' in notice,
                    'missing font license notice')
        else:
            require(fixture['license'] == 'GPL-3.0-or-later' and fixture['notice'] == 'LICENSE',
                    'wrong project-fixture license')
        if fixture['kind'] == 'script-source':
            require(data_path.suffix in ('.py', '.lua'), 'wrong script fixture format')
            data.decode('utf-8') # Provenance/input validation, never execute fixture scripts here.
        if fixture['kind'] == 'contracts':
            datasets[ident] = validate_contract_inputs(_json_bytes(data), oracles)
        fixtures[ident] = fixture
    return fixtures, datasets


def painter_signatures(header):
    """Canonicalize this bounded named-parameter interface, not arbitrary C++."""
    require(isinstance(header, str) and len(header) <= 128 * 1024, 'invalid/oversized painter header')
    text = _cpp_without_comments(header)
    bodies = re.findall(r'\bclass\s+LcPainter\s*\{([^{}]*)\}\s*;', text, re.S)
    require(len(bodies) == 1, 'missing/ambiguous or unsupported LcPainter class')
    body = re.fullmatch(r'\s*public\s*:\s*(.*?)\s*', bodies[0], re.S)
    require(body is not None, 'painter API must be public')
    entries = body[1].split(';')
    require(not entries[-1].strip() and len(entries) <= 130, 'unterminated/oversized painter API')
    signatures, overloads, destructor = [], set(), 0
    for entry in entries[:-1]:
        entry = entry.strip()
        require(0 < len(entry) <= 4096, 'empty/oversized painter declaration')
        if re.fullmatch(r'virtual\s+~LcPainter\s*\(\s*\)\s*=\s*default', entry):
            destructor += 1
            continue
        match = re.fullmatch(r'virtual\s+(.+?(?:\s|[*&]))([A-Za-z_]\w*)\s*\(([^()]*)\)\s*(const\s*)?=\s*0',
                             entry, re.S | re.ASCII)
        require(match is not None, 'unsupported or non-pure painter declaration: ' + entry[:100])
        params, names, default_seen = [], set(), False
        for parameter in _painter_parts(match[3]):
            parts = parameter.split('=')
            require(len(parts) <= 2, 'unsupported painter default')
            if len(parts) == 2:
                require(re.fullmatch(r'(?:[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?|true|false|nullptr)',
                                     parts[1].strip()), 'unsupported painter default')
                default_seen = True
            else:
                require(not default_seen, 'required painter parameter follows default')
            named = re.fullmatch(r'(.+?(?:\s|[*&]))([A-Za-z_]\w*)', parts[0].strip(), re.S | re.ASCII)
            require(named is not None, 'painter parameters must be named')
            require(named[2] not in names, 'duplicate painter parameter name')
            names.add(named[2])
            params.append(_painter_type(named[1]))
        overload = '{}({}){}'.format(match[2], ','.join(params), ' const' if match[4] else '')
        require(overload not in overloads, 'duplicate painter overload')
        overloads.add(overload)
        signatures.append(_painter_type(match[1]) + ' ' + overload)
    require(destructor == 1 and signatures, 'missing/duplicate default virtual destructor or empty painter API')
    return signatures


def _cpp_without_comments(text):
    """Mask comments/literals without joining tokens; inspect every #if branch."""
    text = re.sub(r'\\\r?\n', '', text)
    token = re.compile(
        r'(?:u8|[uUL])?R"(?P<delimiter>[^\s()\\]{0,16})\(.*?\)(?P=delimiter)"'
        r'|(?:u8|[uUL])?"(?:\\.|[^"\\\n])*"'
        r"|(?P<number>\b[0-9](?:[A-Za-z0-9_.]|'[A-Za-z0-9_])*)"
        r"|(?:u8|[uUL])?'(?:\\.|[^'\\\n])*'"
        r'|/\*.*?\*/|//[^\n]*|(?P<invalid>/\*|\*/|R"|["\'])', re.S)

    def mask(match):
        require(match['invalid'] is None, 'unterminated/stray C++ comment or literal')
        return match[0] if match['number'] is not None else ' '

    return token.sub(mask, text)


def _painter_parts(source):
    if not source.strip():
        return []
    parts, start, depth = [], 0, 0
    for index, char in enumerate(source + ','):
        depth += (char == '<') - (char == '>')
        require(0 <= depth <= 8, 'unbalanced/oversized painter template')
        if char == ',' and depth == 0:
            part = source[start:index].strip()
            require(part, 'empty painter parameter/template argument')
            parts.append(part)
            start = index + 1
    require(depth == 0, 'unbalanced painter template')
    return parts


def _painter_type(value, depth=0):
    require(depth <= 8, 'oversized painter type')
    value = re.sub(r'\s+', ' ', value.strip())
    value = re.sub(r'\s*(::|[<>,*&])\s*', r'\1', value)
    value = re.sub(r'>const\b', '> const', value)
    name = r'[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*'
    match = re.fullmatch(r'(const )?((?:unsigned|signed) (?:char|int|long(?: long)?)|long long|' + name +
                         r')(?:<(.+)>)?( const)?((?:\*(?:const)?)*(?:&&|&)?)', value, re.ASCII)
    require(match is not None and not (match[1] and match[4]) and
            match[2] not in ('const', 'volatile', 'unsigned', 'signed', 'auto', 'decltype', 'virtual'),
            'unsupported painter type: ' + value[:100])
    base = match[2]
    if match[3] is not None:
        base += '<' + ','.join(_painter_type(part, depth + 1) for part in _painter_parts(match[3])) + '>'
    return ('const ' if match[1] or match[4] else '') + base + match[5]


def _repo_file(name, root=REPO_ROOT):
    require(isinstance(name, str) and name and '\\' not in name and ':' not in name and
            not name.startswith('/') and not Path(name).is_absolute() and '..' not in Path(name).parts,
            'unsafe repository input path')
    path = (root / name).resolve()
    require(root.resolve() in path.parents and path.is_file() and path.stat().st_size <= 8 * 1024 * 1024,
            'missing/outside/oversized repository input: ' + name)
    return path


def _unique(values, description):
    require(isinstance(values, list) and values and all(isinstance(v, str) and v for v in values) and len(values) == len(set(values)),
            'empty/duplicate ' + description)
    return set(values)


def _fields(obj, required, optional=()):
    require(isinstance(obj, dict) and set(required) <= obj.keys() and
            obj.keys() <= set(required) | set(optional), 'missing/unknown case-manifest fields')


_CONSUMER_ROLES = {
    'lcviewernoqt/documentcanvas.cpp': 'canvas',
    'lcviewernoqt/documentcanvas.h': 'canvas',
    'lcviewernoqt/events/drawevent.h': 'canvas',
    'lcviewernoqt/events/mousemoveevent.h': 'canvas',
    'lcviewernoqt/events/mousereleaseevent.h': 'canvas',
    'lcviewernoqt/painters/createpainter.cpp': 'canvas',
    'lcviewernoqt/painters/createpainter.h': 'canvas',
    'lcviewernoqt/painters/lccairopainter.tcc': 'cairo',
    'lcUI/lcadviewer.cpp': 'widget', 'lcUI/lcadviewer.h': 'widget',
    'lcUI/lcadmodelviewerimpl.cpp': 'widget', 'lcUI/lcadpaperviewerimpl.cpp': 'widget',
    'lcUI/lcadviewerproxy.cpp': 'widget', 'lcUI/main.cpp': 'widget',
    'lcUI/dialogs/aboutdialog.cpp': 'widget',
    'lcUI/lua/guibridge.cpp': 'scripting', 'lcUI/python/pyguibridge.cpp': 'scripting',
    'lcscripting/primitive/customentity.cpp': 'scripting',
    'luacmdinterface/main.cpp': 'cli', 'unittest/rendering/renderingtest.cpp': 'legacy-reference',
    'CMakeLists.txt': 'build',
}
_CPP_SUFFIXES = {'.cpp', '.cc', '.cxx', '.mm', '.h', '.hpp', '.hh', '.hxx', '.tcc'}


def consumer_sources(root=REPO_ROOT):
    """Audit source roles, not renderer behavior; new unknown callers need review."""
    roles = dict(_CONSUMER_ROLES)
    for folder in ('lcviewernoqt/drawitems', 'lcviewernoqt/drawables'):
        for path in (root / folder).rglob('*'):
            if path.is_file() and path.suffix.lower() in _CPP_SUFFIXES:
                roles[path.relative_to(root).as_posix()] = 'drawable'
    for source in roles:
        _repo_file(source, root)
    # This lexical drift guard includes platform #if branches. It is not a C++
    # dependency/alias analysis or a claim that each registered behavior is tested.
    for folder in ('lcUI', 'lcviewernoqt', 'luacmdinterface', 'lcscripting', 'lcadluascript', 'lcadpythonscript'):
        for path in (root / folder).rglob('*'):
            if not path.is_file() or path.suffix.lower() not in _CPP_SUFFIXES:
                continue
            source = path.relative_to(root).as_posix()
            if source == 'lcviewernoqt/painters/lcpainter.h' or source.startswith('lcviewernoqt/painters/opengl/'):
                continue # API register and backend implementation, not consumers.
            code = _cpp_without_comments(_repo_file(source, root).read_text(encoding='utf-8'))
            if re.search(r'\b(?:LcPainter|createOpenGLPainter|createCairoImagePainter)\b', code):
                require(source in roles, 'unclassified painter consumer: ' + source)
    return roles


def validate_source_register(register, root=REPO_ROOT):
    """Validate the manifest's API/source projection, without certifying execution."""
    _fields(register, ('painter_header', 'header_sha256', 'methods', 'consumers'))
    require(register['painter_header'] == 'lcviewernoqt/painters/lcpainter.h', 'wrong painter header')
    header = _repo_file(register['painter_header'], root)
    header_bytes = header.read_bytes()
    require(_digest(register, 'header_sha256') == sha256(header_bytes), 'stale painter header/defaults')
    methods = _unique(register['methods'], 'canonical painter signatures')
    require(methods == set(painter_signatures(header_bytes.decode('utf-8'))), 'canonical painter API drift')
    expected = consumer_sources(root)
    require(isinstance(register['consumers'], list) and 1 <= len(register['consumers']) <= 256, 'invalid consumer inventory')
    consumers, sources = {}, set()
    for consumer in register['consumers']:
        _fields(consumer, ('id', 'source', 'kind'))
        ident, source, kind = (_string(consumer, key) for key in ('id', 'source', 'kind'))
        require(re.fullmatch(r'[a-z][a-z0-9-]{0,79}', ident) and ident not in consumers, 'duplicate/invalid consumer ID')
        _repo_file(source, root)
        require(source not in sources and source in expected, 'duplicate/unknown consumer source: ' + source)
        require(kind == expected[source], 'false consumer role: ' + source)
        consumers[ident] = consumer
        sources.add(source)
    require(sources == expected.keys(), 'drawable/consumer source drift')
    return methods, consumers


def profile_options(profile):
    require(isinstance(profile, str) and profile in _PROFILE_FLAGS, 'unknown build profile')
    options = dict(zip(_FEATURE_OPTIONS.values(), map(bool, _PROFILE_FLAGS[profile])))
    return dict(options, WITH_UNITTESTS=True, WITH_RENDERER_TESTS=options['WITH_RHI_RENDERER'],
                WITH_RENDERING_UNITTESTS=options['WITH_LEGACY_OPENGL_RENDERER'])


def case_configurations(configs):
    """Expand valid relations, rejecting dead fields and duplicate logical tuples."""
    require(isinstance(configs, list) and 1 <= len(configs) <= 32, 'invalid configuration templates')
    expanded, names, logical = {}, set(), set()
    packages = {'appimage': {'LINUX-X64'}, 'snap': {'LINUX-X64'},
                'nsis': {'WIN-X64'}, 'bundle': {'MAC-ARM64', 'MAC-X64'}}
    for config in configs:
        _fields(config, ('id', 'platforms', 'backend', 'modes', 'qpas', 'devices', 'profiles', 'artifact'))
        ident = _identifier(config['id'], 'configuration ID')
        require(ident not in names, 'duplicate configuration ID')
        names.add(ident)
        platforms = _references(config['platforms'], CASE_PLATFORMS.keys(), 'configuration platform')
        modes = _references(config['modes'], {'cpu', 'offscreen', 'widget'}, 'configuration mode')
        profiles = _references(config['profiles'], PROFILE_MODES.keys(), 'configuration profile')
        qpas = _references(config['qpas'], {'none', 'xcb', 'wayland', 'windows', 'cocoa'}, 'configuration QPA')
        devices = _references(config['devices'], {'none', 'software', 'hardware', 'virtual', 'any'}, 'configuration device')
        backend, artifact = _string(config, 'backend'), _string(config, 'artifact')
        require(artifact in {'source', 'install', *packages}, 'unknown configuration artifact')
        used = {key: set() for key in ('platforms', 'modes', 'qpas', 'devices', 'profiles')}
        for platform, mode, qpa, device, profile in itertools.product(
                sorted(platforms), sorted(modes), sorted(qpas), sorted(devices), sorted(profiles)):
            if mode not in PROFILE_MODES[profile]:
                continue  # Relation join, not a promise of unsupported cross-pairs.
            api, native_qpas, native_devices = CASE_PLATFORMS[platform]
            options = profile_options(profile)
            if mode == 'cpu':
                require(artifact == 'source' and qpa == device == 'none' and
                        ((profile == 'cpu' and backend == 'none') or
                         (profile == 'cairo' and backend == 'cairo' and platform == 'LINUX-X64')),
                        'invalid CPU/Cairo configuration')
            elif profile == 'dual-legacy':
                require(backend == 'legacy-opengl' and device == 'any' and qpa in native_qpas and
                        artifact == 'source', 'invalid separate legacy-process configuration')
            else:
                virtual = device == 'virtual'
                if virtual:
                    require((ident, platform, backend, profile, mode, qpa, artifact) in {
                            ('r1-source-virtual-metal', 'MAC-X64', 'metal', 'rhi-offscreen', 'offscreen', 'cocoa', 'source'),
                            ('r1-source-virtual-metal-arm64', 'MAC-ARM64', 'metal', 'rhi-offscreen', 'offscreen', 'cocoa', 'source')},
                            'virtual Metal requires an approved R1 source/offscreen tuple')
                require(backend == api and qpa in native_qpas and (device in native_devices or virtual) and
                        options['WITH_RHI_RENDERER'], 'wrong API/QPA/device for required platform')
                require(not (platform == 'WIN-X64' and options['WITH_LUACMDINTERFACE']),
                        'Windows CLI is not in current scope')
                require(artifact == 'source' or profile in ('rhi-only', 'scripted'),
                        'installed/package profiles require the shipped UI, not dual/test-only hosts')
                require(artifact not in packages or platform in packages[artifact],
                        'unsupported package architecture')
            semantic = (artifact, platform, backend, profile, mode, qpa, device)
            require(semantic not in logical, 'duplicate logical configuration')
            logical.add(semantic)
            key = '/'.join((ident, platform, profile, mode, qpa, device))
            expanded[key] = dict(config_id=ident, platform=platform, backend=backend, profile=profile,
                                 mode=mode, qpa=qpa, device=device, artifact=artifact, expected_options=options)
            for field, value in zip(used, (platform, mode, qpa, device, profile)):
                used[field].add(value)
            require(len(expanded) <= 1024, 'oversized configuration expansion')
        require(all(used[field] == set(config[field]) for field in used),
                'empty/unreachable configuration field')
    _configuration_coverage(expanded)
    return dict(sorted(expanded.items()))


def _configuration_coverage(configs):
    """Logical coverage layers only; no runner availability or D4 acceptance."""
    values = list(configs.values())
    def has(platform, artifact, profile, mode, qpa, devices):
        return any(c['platform'] == platform and c['artifact'] == artifact and c['profile'] == profile and
                   c['mode'] == mode and c['qpa'] == qpa and c['device'] in devices for c in values)
    for platform, (_, qpas, _) in CASE_PLATFORMS.items():
        require(has(platform, 'source', 'cpu', 'cpu', 'none', {'none'}), 'missing CPU platform')
        # Windows has a D4 software-or-trusted-hardware route, not automatic fallback.
        baseline = {'software'} if platform.startswith('LINUX-') else {'hardware'} if platform.startswith('MAC-') else {'software', 'hardware'}
        for qpa in qpas:
            for profile, mode in itertools.product(('rhi-only', 'dual', 'scripted'), ('offscreen', 'widget')):
                require(has(platform, 'source', profile, mode, qpa, baseline), 'missing source functional layer')
            require(has(platform, 'source', 'rhi-offscreen', 'offscreen', qpa, baseline), 'missing UI-OFF layer')
            if platform != 'WIN-X64':
                for profile in ('rhi-cli', 'rhi-cli-python'):
                    require(has(platform, 'source', profile, 'offscreen', qpa, baseline), 'missing supported CLI layer')
            require(has(platform, 'source', 'dual-legacy', 'widget', qpa, {'any'}), 'missing explicit legacy process')
            require(has(platform, 'source', 'rhi-only', 'widget', qpa, {'hardware'}), 'missing native hardware candidate')
            for profile, mode in itertools.product(('rhi-only', 'scripted'), ('offscreen', 'widget')):
                require(has(platform, 'install', profile, mode, qpa, {'software', 'hardware'}), 'missing installed runtime layer')
            artifacts = ('appimage', 'snap') if platform == 'LINUX-X64' else ('nsis',) if platform == 'WIN-X64' else ('bundle',) if platform.startswith('MAC-') else ()
            for artifact, mode in itertools.product(artifacts, ('offscreen', 'widget')):
                require(has(platform, artifact, 'scripted', mode, qpa, {'software', 'hardware'}), 'missing final package layer')
        require(any(has(platform, 'source', 'rhi-only', 'offscreen', qpa, {'hardware'}) for qpa in qpas),
                'missing hardware offscreen candidate')
    require(has('LINUX-X64', 'source', 'cairo', 'cpu', 'none', {'none'}), 'missing optional Cairo layer')


def validate_case_inputs(path=CASE_PATH, root=REPO_ROOT):
    """M1/M2 source, input and binding contracts; no tuple/execution acceptance."""
    manifest_bytes = _bounded_bytes(path, 2 * 1024 * 1024)
    manifest = _json_bytes(manifest_bytes)
    _fields(manifest, ('schema_version', 'evidence_kind', 'painter_header', 'header_sha256', 'methods',
                       'consumers', 'fixtures', 'configurations', 'oracles', 'cases', 'workloads', 'notes'))
    require(type(manifest['schema_version']) is int and manifest['schema_version'] == 2 and
            manifest['evidence_kind'] == 'renderer-case-requirements', 'wrong case-manifest schema/scope')
    methods, consumers = validate_source_register(
        {key: manifest[key] for key in ('painter_header', 'header_sha256', 'methods', 'consumers')}, root)
    _unique(manifest['notes'], 'scope notes')
    oracles = _case_oracles(manifest['oracles'])
    fixtures, datasets = _case_fixtures(manifest['fixtures'], root, oracles)
    configs = manifest['configurations']
    require(isinstance(configs, list) and 1 <= len(configs) <= 32, 'invalid configuration inventory')
    config_ids, config_records = set(), {}
    for config in configs:
        _fields(config, ('id', 'platforms', 'backend', 'modes', 'qpas', 'devices', 'profiles', 'artifact'))
        ident = _identifier(config['id'], 'configuration ID')
        require(ident not in config_ids, 'duplicate configuration ID')
        config_ids.add(ident)
        config_records[ident] = config
    features = {'F' + str(i) for i in range(1, 13)}
    seen_methods, seen_features, seen_consumers, used_fixtures, used_datasets, used_oracles = (set() for _ in range(6))
    require(isinstance(manifest['cases'], list) and 1 <= len(manifest['cases']) <= 128, 'invalid case inventory')
    cases, requirements, tests = {}, [], set()
    for case in manifest['cases']:
        _fields(case, ('id', 'stage', 'features', 'methods', 'consumers', 'requires', 'configurations', 'profiles', 'modes',
                       'devices', 'fixture', 'dataset', 'assets', 'decisions', 'binding', 'variants'))
        ident = _identifier(case['id'], 'case ID')
        require(ident not in cases, 'duplicate case ID')
        cases[ident] = case
        require(_string(case, 'stage') in {'R' + str(i) for i in range(1, 8)} and
                _string(case, 'binding') in ('proposed-gtest', 'proposed-runner-check'),
                'production cases must be proposed, not spike/execution evidence')
        seen_methods.update(_references(case['methods'], methods, 'case method', optional=True))
        seen_features.update(_references(case['features'], features, 'case feature'))
        seen_consumers.update(_references(case['consumers'], consumers.keys(), 'case consumer'))
        groups = _references(case['configurations'], config_ids, 'case configuration')
        require(case['binding'] != 'proposed-gtest' or
                all(config_records[g]['artifact'] == 'source' for g in groups),
                'installed/package evidence cannot require a shipped GTest binary')
        _references(case['profiles'], PROFILE_MODES.keys(), 'case profile')
        _references(case['modes'], {'cpu', 'offscreen', 'widget'}, 'case mode')
        _references(case['devices'], {'none', 'software', 'hardware', 'virtual', 'any'}, 'case device')
        if 'virtual' in case['devices'] or {'r1-source-virtual-metal','r1-source-virtual-metal-arm64'}.intersection(groups):
            require(case['id'] == 'rhi-build-options' and case['stage'] == 'R1' and
                    case['binding'] == 'proposed-gtest' and case['dataset'] == 'rhi-build-options',
                    'virtual Metal cannot qualify another stage/case')
        _references(case['decisions'], {'D' + str(i) for i in range(1, 7)}, 'case decision')
        _references(case['requires'], _FEATURE_OPTIONS.keys(), 'case prerequisite', optional=True)
        fixture, dataset = _string(case, 'fixture'), _string(case, 'dataset')
        require(fixture in datasets and dataset in datasets[fixture], 'missing case input dataset')
        used_fixtures.add(fixture)
        assets = _references(case['assets'], fixtures.keys(), 'case asset', optional=True)
        used_fixtures.update(assets)
        used_datasets.add((fixture, dataset))
        require(isinstance(case['variants'], list) and 3 <= len(case['variants']) <= 16, 'invalid case variants')
        names = set()
        for variant in case['variants']:
            _fields(variant, ('id', 'test', 'expect', 'oracles'), ('reason',))
            name, test = _identifier(variant['id'], 'case variant'), _string(variant, 'test')
            require(name not in names and name in datasets[fixture][dataset], 'duplicate/missing variant input')
            names.add(name)
            require(re.fullmatch(r'[A-Za-z_][A-Za-z0-9_/]*\.[A-Za-z_][A-Za-z0-9_/]*', test) and
                    'DISABLED_' not in test and not re.search(r'r0|spike', test, re.I) and test not in tests,
                    'duplicate/disabled/wildcard/spike test binding')
            tests.add(test)
            body = datasets[fixture][dataset][name]
            input_assets = {v['value'] for v in body['inputs'].values() if v['type'] == 'asset'}
            require(input_assets <= assets, 'undeclared/unknown input asset reference')
            required_oracles = {a['oracle'] for a in body['assertions']}
            require(_references(variant['oracles'], oracles.keys(), 'variant oracle') == required_oracles,
                    'variant/assertion oracle disagreement')
            used_oracles.update(required_oracles)
            outcome = _string(variant, 'expect')
            require(outcome in ('success', 'diagnostic'), 'unknown expected outcome')
            diagnostic = [a['expected'] for a in body['assertions'] if a['observed'] == 'diagnostic']
            if outcome == 'diagnostic':
                reason = _identifier(variant.get('reason'), 'diagnostic reason')
                require(diagnostic == [dict(type='text', unit='none', value=reason)], 'missing/mismatched diagnostic assertion')
            else:
                require(not diagnostic and 'reason' not in variant and
                        not any(v['type'] == 'nonfinite' for v in body['inputs'].values()),
                        'successful case carries diagnostic or invalid-domain input')
            requirements.append(dict(id=ident + '.' + name, test=test, binding=case['binding'], stage=case['stage'],
                                     fixture=fixture, dataset=dataset, variant=name, expect=outcome,
                                     oracles=sorted(required_oracles), assets=sorted(assets),
                                     requires=sorted(case['requires']),
                                     **({'reason': variant['reason']} if outcome == 'diagnostic' else {})))
        require(names == datasets[fixture][dataset].keys(), 'missing/orphan case dataset variants')
    require(seen_methods == methods and seen_features == features and seen_consumers == consumers.keys(),
            'unmapped painter method, feature or consumer')
    workloads = manifest['workloads']
    require(isinstance(workloads, list) and 3 <= len(workloads) <= 16, 'missing/bounded workload requirements')
    workload_ids = set()
    for workload in workloads:
        _fields(workload, ('id', 'case', 'fixture', 'dataset', 'variants', 'samples', 'warmups', 'oracles'))
        ident, case_id = _identifier(workload['id'], 'workload ID'), _string(workload, 'case')
        require(ident not in workload_ids and case_id in cases, 'duplicate/unknown workload case')
        workload_ids.add(ident)
        case = cases[case_id]
        require(workload['fixture'] == case['fixture'] and workload['dataset'] == case['dataset'], 'workload/case input mismatch')
        variants = _references(workload['variants'], {v['id'] for v in case['variants']}, 'workload variant')
        require(all(v['expect'] == 'success' for v in case['variants'] if v['id'] in variants),
                'diagnostic case cannot be a successful performance sample')
        _integer(workload, 'samples', 1, 1000000)
        _integer(workload, 'warmups', 0, 1000000)
        metrics = _references(workload['oracles'], oracles.keys(), 'workload oracle')
        require(all(oracles[o]['metric'] in ('memory-growth', 'interaction-latency', 'interaction-time',
                                            'cold-open-latency', 'idle-submissions') for o in metrics),
                'workload requires a performance/idle budget, not a geometry oracle')
        require(set(case['devices']) == {'hardware'}, 'software cannot qualify a performance workload')
        used_oracles.update(metrics)
    require(used_fixtures == fixtures.keys(), 'unreferenced fixture')
    require(used_datasets == {(f, d) for f, ds in datasets.items() for d in ds}, 'orphan input dataset')
    require(used_oracles == oracles.keys(), 'unreferenced oracle')
    return dict(manifest=manifest, manifest_sha256=sha256(manifest_bytes),
                methods=methods, consumers=consumers, fixtures=fixtures,
                datasets=datasets, oracles=oracles, requirements=requirements)


_FEATURE_DEADLINES = {**{'F' + str(i): 2 for i in (1, 2, 3, 4, 7, 8)},
                      **{'F' + str(i): 3 for i in (5, 6, 9, 11, 12)}, 'F10': 4}
_CONSUMER_DEADLINES = {'build': 1, 'drawable': 3, 'canvas': 3, 'widget': 3,
                       'scripting': 3, 'cli': 4, 'cairo': 4, 'legacy-reference': 4}


def validate_case_manifest(path=CASE_PATH, root=REPO_ROOT):
    """Validate complete logical requirements; never certify native execution."""
    inputs = validate_case_inputs(path, root)
    manifest, consumers = inputs['manifest'], inputs['consumers']
    configs = case_configurations(manifest['configurations'])
    cases = {case['id']: case for case in manifest['cases']}
    by_case, used_configs, requirements, references = {}, set(), [], 0
    for case in cases.values():
        fields = ('configurations', 'profiles', 'modes', 'devices')
        wanted = {key: set(case[key]) for key in fields}
        selected = {key: value for key, value in configs.items() if value['config_id'] in wanted['configurations'] and
                    value['profile'] in wanted['profiles'] and value['mode'] in wanted['modes'] and
                    value['device'] in wanted['devices']}
        require(selected and all({v[{'configurations': 'config_id', 'profiles': 'profile', 'modes': 'mode',
                                     'devices': 'device'}[field]] for v in selected.values()} == wanted[field]
                                 for field in fields), 'empty/unreachable case selection: ' + case['id'])
        needed = set(case['requires'])
        datasets = inputs['datasets'][case['fixture']][case['dataset']]
        successes = {variant['id'] for variant in case['variants'] if variant['expect'] == 'success'}
        python_input = any(v.get('type') == 'text' and v.get('value') == 'python'
                           for name, body in datasets.items() if name in successes
                           for key, v in body['inputs'].items() if key == 'language')
        python_asset = bool(successes) and any(inputs['fixtures'][asset]['kind'] == 'script-source' and
                           inputs['fixtures'][asset]['path'].endswith('.py') for asset in case['assets'])
        require(not (python_input or python_asset) or 'python' in needed, 'missing explicit Python prerequisite')
        for value in selected.values():
            require(all(value['expected_options'][_FEATURE_OPTIONS[feature]] for feature in needed),
                    'case prerequisite conflicts with build profile: ' + case['id'])
            require(not ('rhi' in needed and value['backend'] not in ('vulkan', 'metal')) and
                    not ('legacy' in needed and value['backend'] != 'legacy-opengl') and
                    not ('cairo' in needed and value['backend'] != 'cairo'), 'case requires a different active backend')
        by_case[case['id']] = set(selected)
        used_configs.update(selected)
    require(used_configs == configs.keys(), 'configuration has no required case')
    for required in inputs['requirements']:
        case_id = required['id'].rsplit('.', 1)[0]
        selected = sorted(by_case[case_id])
        references += len(selected)
        require(references <= 65536, 'oversized case/configuration selection')
        requirements.append(dict(required, configurations=selected))
    _case_coverage(cases, by_case, configs, consumers, inputs['methods'])
    return dict(inputs, configurations=configs, requirements=sorted(requirements, key=lambda r: r['id']))


def _case_coverage(cases, by_case, configs, consumers, methods):
    """Check deadlines/ordinary-empty-limit assignment, not runtime availability."""
    for key, config in configs.items():
        if config['artifact'] != 'source':
            continue  # Runner smoke, not all source GTests repeated in a package.
        options, mode, backend = config['expected_options'], config['mode'], config['backend']
        matched = [c for c in cases.values() if key in by_case[c['id']]]
        if config['device'] == 'virtual':
            require([c['id'] for c in matched] == ['rhi-build-options'], 'virtual Metal requires its R1 cases only')
            continue # An additive source-CI tuple cannot replace any mandatory hardware coverage.
        ordinary_success = [c for c in matched if any(v['id'] == 'ordinary' and v['expect'] == 'success'
                                                    for v in c['variants'])]
        if backend == 'legacy-opengl':
            require(any(int(c['stage'][1]) <= 4 for c in ordinary_success),
                    'missing successful legacy compatibility requirement')
        if backend in ('vulkan', 'metal'):
            for method in methods:
                deadline = 4 if method == 'unsigned char* data()' else 3
                require(any(method in c['methods'] and int(c['stage'][1]) <= deadline for c in ordinary_success),
                        'missing timely painter method for ' + key + ': ' + method)
            for feature, deadline in _FEATURE_DEADLINES.items():
                if feature == 'F11' and not (options['WITH_LUACMDINTERFACE'] or mode == 'widget'):
                    continue
                require(any(feature in c['features'] and int(c['stage'][1]) <= deadline for c in ordinary_success),
                        'missing timely feature for ' + key + ': ' + feature)
        for ident, consumer in consumers.items():
            kind = consumer['kind']
            applies = ((kind == 'build' and backend == 'none') or (kind == 'cairo' and backend == 'cairo') or
                       (kind == 'widget' and backend in ('vulkan', 'metal') and mode == 'widget') or
                       (kind == 'cli' and options['WITH_LUACMDINTERFACE']) or
                       (kind == 'scripting' and backend in ('vulkan', 'metal') and mode == 'widget' and
                        ('/python/' not in consumer['source'] or options['WITH_PYTHONSCRIPT'])) or
                       (kind in ('drawable', 'canvas', 'legacy-reference') and backend in ('vulkan', 'metal')))
            if applies:
                for variant in ('ordinary', 'empty', 'limit'):
                    require(any(ident in c['consumers'] and int(c['stage'][1]) <= _CONSUMER_DEADLINES[kind] and
                                any(v['id'] == variant and (variant != 'ordinary' or v['expect'] == 'success')
                                    for v in c['variants']) for c in matched),
                            'missing timely consumer/' + variant + ' for ' + key + ': ' + ident)


def select_case_requirements(inventory, stage, configuration):
    require(isinstance(stage, str) and re.fullmatch(r'R[1-7]', stage), 'unknown production requirement stage')
    require(isinstance(configuration, str) and configuration in inventory['configurations'], 'unknown configuration')
    config = inventory['configurations'][configuration]
    number = int(stage[1])
    require(config['device'] != 'virtual' or number == 1, 'virtual Metal source CI is R1 only')
    require(config['mode'] != 'widget' or number >= 3, 'widget requirements start at R3')
    require(config['artifact'] == 'source' or number >= 5, 'installed/package requirements start at R5')
    require(number != 7 or not config['expected_options']['WITH_LEGACY_OPENGL_RENDERER'],
            'retirement excludes dual/legacy configurations')
    selected = [dict((k, v) for k, v in row.items() if k != 'configurations')
                for row in inventory['requirements'] if configuration in row['configurations'] and
                int(row['stage'][1]) <= number]
    require(selected, 'no requirements for requested stage/configuration')
    return dict(schema_version=1, evidence_kind='renderer-case-selection', stage=stage,
                configuration_id=configuration, configuration=config, execution_evidence=False,
                manifest_sha256=inventory['manifest_sha256'], header_sha256=inventory['manifest']['header_sha256'],
                fixtures=inventory['fixtures'], oracles=inventory['oracles'], requirements=selected)


def case_inventory_summary(inventory):
    return dict(schema_version=1, evidence_kind='renderer-case-inventory', execution_evidence=False,
                manifest_sha256=inventory['manifest_sha256'], methods=len(inventory['methods']),
                consumers=len(inventory['consumers']), cases=len(inventory['manifest']['cases']),
                variants=len(inventory['requirements']), configurations=sorted(inventory['configurations']),
                profiles={name: profile_options(name) for name in sorted(_PROFILE_FLAGS)},
                stages={stage: sum(row['stage'] == stage for row in inventory['requirements'])
                        for stage in ('R' + str(i) for i in range(1, 8))})


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def _pairs(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, "duplicate JSON field: " + key)
        result[key] = value
    return result


def read_json(path):
    return _json_bytes(_bounded_bytes(path, 2 * 1024 * 1024))


def _bounded_bytes(path, limit):
    with path.open('rb') as stream:
        data = stream.read(limit + 1)
    require(len(data) <= limit, 'oversized repository input/report')
    return data


def _json_bytes(data):
    require(len(data) <= 2 * 1024 * 1024, 'oversized JSON report')
    return json.loads(data.decode('utf-8'), object_pairs_hook=_pairs,
                      parse_constant=lambda value: require(False, "nonfinite JSON: " + value))


def _string(obj, name):
    value = obj.get(name)
    require(isinstance(value, str) and value.strip(), "missing/invalid " + name)
    return value


def _integer(obj, name, low, high):
    value = obj.get(name)
    require(type(value) is int and low <= value <= high, "missing/invalid " + name)
    return value


def _digest(obj, name):
    value = _string(obj, name)
    require(re.fullmatch(r"[0-9a-f]{64}", value), "invalid SHA256: " + name)
    return value


def _object(parent, name):
    value = parent.get(name)
    require(isinstance(value, dict), "missing/invalid object: " + name)
    return value


def _artifact(root, name):
    require(isinstance(name, str) and name and Path(name).name == name and
            "/" not in name and "\\" not in name, "artifact must be a local filename")
    path = root / name
    require(path.resolve().parent == root.resolve(), "artifact escapes evidence directory")
    require(path.is_file() and path.stat().st_size <= 80 * 1024 * 1024, "missing/oversized artifact")
    return path.read_bytes()


def decode_png(data):
    """Decode Qt's noninterlaced 8-bit RGB/RGBA PNG, checking framing and CRCs."""
    require(data.startswith(b"\x89PNG\r\n\x1a\n"), "not a PNG")
    pos, compressed, header, ended = 8, bytearray(), None, False
    while pos < len(data):
        require(pos + 12 <= len(data), "truncated PNG chunk")
        length = struct.unpack_from(">I", data, pos)[0]
        kind = data[pos + 4:pos + 8]
        payload = data[pos + 8:pos + 8 + length]
        require(pos + length + 12 <= len(data), "truncated PNG data")
        crc = struct.unpack_from(">I", data, pos + length + 8)[0]
        require(zlib.crc32(kind + payload) & 0xffffffff == crc, "PNG CRC mismatch")
        if kind == b"IHDR":
            require(header is None and pos == 8 and length == 13, "invalid PNG header")
            header = struct.unpack(">IIBBBBB", payload)
        elif kind == b"IDAT":
            compressed.extend(payload)
        elif kind == b"IEND":
            require(length == 0 and pos + 12 == len(data), "invalid PNG end")
            ended = True
        pos += length + 12
    require(header is not None and ended, "incomplete PNG")
    width, height, depth, color, compression, filtering, interlace = header
    require(1 <= width <= 4096 and 1 <= height <= 4096 and depth == 8 and
            color in (2, 6) and compression == filtering == interlace == 0, "unsupported PNG format")
    bpp = 4 if color == 6 else 3
    stride = width * bpp
    size = height * (stride + 1)
    decoder = zlib.decompressobj()
    decoded = decoder.decompress(bytes(compressed), size + 1)
    require(len(decoded) == size and decoder.eof and not decoder.unused_data, "invalid PNG scanline size")
    output, previous = bytearray(), bytearray(stride)
    for y in range(height):
        offset = y * (stride + 1)
        method, row = decoded[offset], bytearray(decoded[offset + 1:offset + 1 + stride])
        require(method in range(5), "invalid PNG filter")
        for i in range(stride):
            a, b = row[i - bpp] if i >= bpp else 0, previous[i]
            c = previous[i - bpp] if i >= bpp else 0
            if method == 1:
                predictor = a
            elif method == 2:
                predictor = b
            elif method == 3:
                predictor = (a + b) // 2
            elif method == 4:
                p = a + b - c
                distances = (abs(p - a), abs(p - b), abs(p - c))
                predictor = (a, b, c)[distances.index(min(distances))]
            else:
                predictor = 0
            row[i] = (row[i] + predictor) & 255
        if bpp == 4:
            output.extend(row)
        else:
            for i in range(0, stride, 3):
                output.extend(row[i:i + 3] + b"\xff")
        previous = row
    return width, height, bytes(output)


def normalize_arch(value):
    return {"aarch64": "arm64", "amd64": "x86_64", "x64": "x86_64"}.get(value.lower(), value.lower())


def source_inputs():
    paths = {path for pattern in ("*.cpp", "*.h", "*.vert", "*.frag", "*.json", "*.in", "*.cmake", "CMakeLists.txt")
             for path in SPIKE_ROOT.rglob(pattern) if path.is_file()}
    return "".join(sha256(path.read_bytes()) + "  " + path.relative_to(SPIKE_ROOT).as_posix() + "\n"
                   for path in sorted(paths, key=lambda path: path.relative_to(SPIKE_ROOT).as_posix()))


def validate_capabilities(caps):
    minimum = _integer(caps, "min_texture_size", 1, 16)
    maximum = _integer(caps, "max_texture_size", 256, 2 ** 31 - 1)
    _integer(caps, "max_uniform_buffer_range", 64, 2 ** 31 - 1)
    require(caps.get("rgba8") is True and caps.get("rgba8_render_target") is True and
            isinstance(caps.get("sample_counts"), list) and
            all(type(value) is int and value > 0 for value in caps["sample_counts"]) and
            1 in caps["sample_counts"], "unsupported required capabilities")
    return minimum, maximum


def _validate_identity(report, backend, device, mode, expected_revision, expected_arch,
                       expected_qpa, background, scenario, kind):
    require(isinstance(report, dict), "report is not an object")
    require(type(report.get("schema_version")) is int and report["schema_version"] == 3, "unsupported schema")
    require(report.get("evidence_kind") == kind, "this validator only accepts the requested R0 spike evidence kind")
    fixture = read_json(FIXTURE_PATH)
    require(report.get("fixture") == fixture["id"], "wrong fixture")
    build = _object(report, "build")
    revision = _string(build, "source_revision")
    require(re.fullmatch(r"[0-9a-f]{40}", revision), "source revision must be a full SHA")
    require(not expected_revision or revision == expected_revision, "wrong source revision")
    require(type(build.get("source_dirty")) is bool, "missing/invalid source dirty state")
    _digest(build, "source_diff_sha256")
    require(_digest(build, "source_inputs_sha256") == sha256(_string(build, "source_inputs").encode()),
            "source-input manifest digest mismatch")
    for line in build["source_inputs"].splitlines():
        require(re.fullmatch(r"[0-9a-f]{64}  [^\r\n]+", line), "invalid source-input entry")
        name = line[66:]
        require(not Path(name).is_absolute() and ".." not in Path(name).parts and "\\" not in name,
                "unsafe source-input path")
    require(build["source_inputs"] == source_inputs(), "stale or incomplete spike source-input manifest")
    qt = _string(build, "qt_version")
    require(re.fullmatch(r"6\.\d+\.\d+", qt) and int(qt.split(".")[1]) >= 8, "unsupported Qt version")
    require(build.get("compile_qt_version") == qt and build.get("qsb_version") == "qsb " + qt,
            "build Qt/private/qsb version mismatch")
    _digest(build, "qsb_sha256")
    require(_digest(build, "fixture_sha256") == sha256(FIXTURE_PATH.read_bytes()), "stale fixture")
    shaders = _object(build, "shaders")
    require(set(shaders) == {"scene.vert.qsb", "scene.frag.qsb"}, "missing/unexpected shader package")
    for name in shaders:
        _digest(shaders, name)
    _string(build, "compiler")
    _string(build, "configuration")
    _string(build, "generator")
    require(isinstance(build.get("deployment_target"), str), "invalid deployment target")
    build_arch = normalize_arch(_string(build, "target_arch"))
    options = _object(build, "options")
    require(options.get("standalone_spike") is True and options.get("production_rhi") is False,
            "wrong effective build options")
    require(type(options.get("widget")) is bool and (mode != "widget" or options["widget"]), "widget not built")

    runtime = _object(report, "runtime")
    require(backend in ("metal", "vulkan") and runtime.get("requested_api") == backend and
            runtime.get("actual_api") == backend, "requested/actual API mismatch or fallback")
    require(runtime.get("mode") == mode and mode in ("widget", "offscreen"), "execution-mode mismatch")
    require(background in ("opaque", "transparent") and runtime.get("background") == background,
            "requested/observed background mismatch")
    require(scenario in ("standard", "lifecycle", "recovery", "reparent") and runtime.get("scenario") == scenario and
            (scenario != "recovery" or mode == "offscreen") and (scenario != "reparent" or mode == "widget"),
            "requested/observed scenario mismatch")
    require(runtime.get("qt_version") == qt, "runtime/build Qt provider mismatch")
    agl_compat = options.get("legacy_agl_link_workaround", False)
    require(type(agl_compat) is bool and (not agl_compat or
            (runtime.get("os") == "macos" and backend == "metal" and 8 <= int(qt.split(".")[1]) < 10)),
            "unqualified legacy Qt AGL compatibility option")
    _string(runtime, "device_name")
    classification = _string(runtime, "device_class")
    require(classification in ("software", "hardware", "virtual", "unknown"), "invalid device classification")
    require(device == "any" or classification == device, "wrong or unclassified device")
    arch = normalize_arch(_string(runtime, "architecture"))
    require(arch == build_arch and (not expected_arch or arch == normalize_arch(expected_arch)),
            "runtime/build/required architecture mismatch")
    _string(runtime, "os")
    qpa = _string(runtime, "qpa")
    require(expected_qpa is None or qpa == expected_qpa, "observed QPA does not match required session")
    require(mode != "widget" or qpa not in ("offscreen", "minimal"), "unqualified widget QPA")
    require(runtime.get("format") == "RGBA8" and type(runtime.get("sample_count")) is int and
            runtime["sample_count"] == 1, "unexpected target format/sample count")
    alpha = "premultiplied-blending-straight-readback" if background == "transparent" else "opaque-premultiplied-blending"
    require(runtime.get("alpha") == alpha and
            runtime.get("color_space") == "SDR-unorm-no-sRGB-transfer", "unexpected graphics contract")
    caps = _object(runtime, "capabilities")
    minimum, maximum = validate_capabilities(caps)
    # Optional driver version/zero IDs are intentionally not hardware classifiers.
    require("driver_version" in runtime and (runtime["driver_version"] is None or
            isinstance(runtime["driver_version"], str)), "invalid driver availability metadata")
    _string(runtime, "metadata_note")
    for name in ("vendor_id", "device_id"):
        require(re.fullmatch(r"\d+", _string(runtime, name)), "invalid optional device ID")
    return runtime, qt, arch, qpa, minimum, maximum


def validate_failure(path, backend, device, point, expected_revision=None, expected_arch=None,
                     expected_dpr=None, expected_qpa=None, background="opaque"):
    require(point in ("render", "readback", "shutdown"), "unsupported expected failure")
    report = read_json(path)
    runtime, _, _, _, _, _ = _validate_identity(report, backend, device, "widget", expected_revision,
        expected_arch, expected_qpa, background, "standard", "r0-spike-failure")
    require(report.get("outputs") == [] and report.get("errors") == ["Injected widget failure: " + point],
            "expected failure accepted output or had an unrelated/secondary error")
    fault = _object(report, "failure")
    require(fault.get("requested") == fault.get("reached") == point and
            fault.get("injection_method") == "host-terminal-result-substitution" and
            fault.get("physical_device_loss_observed") is False, "wrong/unreached injection or physical-loss overclaim")
    for name in ("windows_destroyed", "qt_shutdown_complete", "all_retired_expired", "success_report_blocked",
                 "success_image_blocked", "zero_exit_requested_after_failure", "terminal_latch_survived"):
        require(fault.get(name) is True, "incomplete failure/finalization: " + name)
    require(fault.get("retirement_release_boundary") == "after-QApplication-destruction", "unsafe retirement boundary")
    retained = _integer(fault, "retained_results", 0, 2)
    _integer(fault, "released_results", retained, retained)
    actual = _integer(fault, "actual_finish_result", -1, 0) # Native failed drains are not injected-test success.
    effective = _integer(fault, "effective_finish_result", -1, 1)
    for name in ("pending_at_injection", "readback_completed_before_drain", "readback_completed_after_drain"):
        require(type(fault.get(name)) is bool, "invalid failure completion observation: " + name)
    require(fault["pending_at_injection"] is (point == "readback") and
            fault.get("capture_completed_at_release") is (point != "render"), "wrong capture completion/pending observation")
    if point == "shutdown":
        require(actual == 0 and effective == 1 and retained == 1 and
                fault["readback_completed_before_drain"] and fault["readback_completed_after_drain"],
                "shutdown substitution did not follow a real completed successful drain")
    else:
        require(effective == actual and retained == 0 and
                (actual != -1 or not (fault["readback_completed_before_drain"] or fault["readback_completed_after_drain"])) and
                (actual != 0 or fault["readback_completed_after_drain"]), "inconsistent actual drain/retirement")
    view = _object(fault, "view")
    require(view.get("id") == "fault-view" and view.get("exposed") is True, "injection did not reach an exposed view")
    _integer(view, "owner_generation", 1, 1)
    generation = _integer(view, "device_generation", 1, 1)
    frames = _integer(view, "rendered_frames", 0 if point == "render" else 1, 2 ** 31 - 1)
    submitted = _integer(view, "submitted_frames", 1 if point == "shutdown" else 0, frames)
    _integer(view, "last_submitted_frame", submitted, frames)
    _integer(view, "exposed_rendered_frames", 0 if point == "render" else 1, frames)
    capture = _integer(view, "capture_generation", 0 if point == "render" else 1, 0 if point == "render" else 2 ** 31 - 1)
    _integer(view, "readback_frame", 0 if not capture else 1, frames)
    _integer(view, "readback_device_generation", generation if capture else 0, generation if capture else 0)
    dpr = view.get("dpr")
    require(type(dpr) in (int, float) and math.isfinite(dpr) and 0 < dpr <= 8 and
            (expected_dpr is None or math.isclose(dpr, expected_dpr, rel_tol=0, abs_tol=0.001)), "wrong failure DPR")
    info = _object(view, "device_info")
    for key in ("actual_api", "device_name", "device_class", "qt_version", "architecture", "qpa", "capabilities"):
        require(info.get(key) == runtime.get(key), "failure view identity differs from initialized device: " + key)
    return ["expected-failure:" + point, "terminal:success-blocked", "terminal:cleanup-complete"]


def validate_report(path, backend, device, mode, expected_revision=None, expected_arch=None,
                    expected_dpr=None, expected_qpa=None, background="opaque", scenario="standard"):
    report = read_json(path)
    runtime, qt, arch, qpa, minimum, maximum = _validate_identity(report, backend, device, mode,
        expected_revision, expected_arch, expected_qpa, background, scenario, "r0-spike")
    fixture = read_json(FIXTURE_PATH)
    views = runtime.get("views")
    count = (8 if scenario == "reparent" else 2) if mode == "widget" else {"standard": 1, "lifecycle": 3, "recovery": 4}[scenario]
    require(isinstance(views, list) and len(views) == count, "missing/extra views")
    view_ids, view_limits = set(), {}
    for view in views:
        require(isinstance(view, dict), "invalid view")
        ident = _string(view, "id")
        require(ident not in view_ids, "duplicate view")
        view_ids.add(ident)
        frames = _integer(view, "rendered_frames", 1, 2 ** 31 - 1)
        _integer(view, "submitted_frames", 1, frames)
        submitted = _integer(view, "last_submitted_frame", 1, frames)
        _integer(view, "readback_frame", 1, submitted)
        _integer(view, "exposed_rendered_frames", 1 if mode == "widget" else 0, frames)
        _integer(view, "owner_generation", 1, 8)
        dpr = view.get("dpr")
        require(type(dpr) in (int, float) and math.isfinite(dpr) and 0 < dpr <= 8, "invalid DPR")
        require(expected_dpr is None or math.isclose(dpr, expected_dpr, rel_tol=0, abs_tol=0.001),
                "observed DPR does not match required DPR")
        require(type(view.get("exposed")) is bool and (mode != "widget" or view["exposed"]), "unexposed widget")
        if scenario == "reparent":
            info = _object(view, "device_info")
            require(info.get("actual_api") == backend and info.get("qt_version") == qt and
                    normalize_arch(_string(info, "architecture")) == arch and info.get("qpa") == qpa,
                    "reparent view changed API/provider/architecture/session")
            _string(info, "device_name")
            kind = _string(info, "device_class")
            require(kind in ("software", "hardware", "virtual", "unknown") and (device == "any" or kind == device),
                    "wrong reparent device classification")
            view_limits[ident] = validate_capabilities(_object(info, "capabilities"))
    require(view_ids == {"view-" + str(i) for i in range(count)}, "unexpected view IDs")
    if mode == "widget":
        require(runtime.get("resized") is True, "resize proof missing")

    outputs = report.get("outputs")
    expected_ids = view_ids | ({"composition"} if mode == "widget" else set())
    require(isinstance(outputs, list) and len(outputs) == len(expected_ids), "missing/extra outputs")
    output_ids, checked = set(), []
    for output in outputs:
        require(isinstance(output, dict), "invalid output")
        ident = _string(output, "id")
        require(ident in expected_ids and ident not in output_ids, "wrong/duplicate output ID")
        output_ids.add(ident)
        width, height = _integer(output, "width", 1, 4096), _integer(output, "height", 1, 4096)
        low, high = view_limits.get(ident, (minimum, maximum))
        require(ident == "composition" or low <= min(width, height) <= max(width, height) <= high,
                "output exceeds qualified device texture limits")
        output_alpha = "straight" if background == "transparent" and ident != "composition" else "opaque"
        require(output.get("stride") == width * 4 and output.get("channels") == "RGBA" and
                output.get("origin") == "top-left" and output.get("alpha") == output_alpha, "invalid pixel layout")
        png, rgba = _artifact(path.parent, output.get("image")), _artifact(path.parent, output.get("rgba"))
        require(sha256(png) == _digest(output, "image_sha256") and sha256(rgba) == _digest(output, "rgba_sha256"),
                "artifact hash mismatch")
        require(len(rgba) == width * height * 4 and decode_png(png) == (width, height, rgba),
                "PNG/readback dimensions or content mismatch")
        if output_alpha == "opaque":
            require(all(alpha == 255 for alpha in rgba[3::4]), "opaque output contains nonopaque pixels")
        if ident != "composition":
            for point in fixture["landmarks"]:
                x, y = int(point["x"] * width / fixture["width"]), int(point["y"] * height / fixture["height"])
                pixel = rgba[(y * width + x) * 4:(y * width + x + 1) * 4]
                expected = point.get("transparent_rgba", point["rgba"]) if background == "transparent" else point["rgba"]
                require(all(abs(a - b) <= fixture["tolerance"] for a, b in zip(pixel, expected)),
                        ident + ": wrong landmark " + point["id"] + ": " + str(list(pixel)))
                checked.append(ident + ":" + point["id"])
        else:
            # The ordinary QLabel has a fixed colored background, distinct from the GPU fixture.
            pixels = sum(rgba[i:i + 4] == bytes((192, 32, 64, 255)) for i in range(0, len(rgba), 4))
            require(pixels >= 100, "ordinary widget composition missing")
            checked.append("composition:ordinary-widget")
    if scenario == "lifecycle":
        checked.extend(validate_lifecycle(runtime, mode, views, backend, device))
    elif scenario == "recovery":
        checked.extend(validate_recovery(runtime, views, backend, device))
    elif scenario == "reparent":
        checked.extend(validate_reparent(runtime, views))
    return checked


def validate_reparent(runtime, views):
    reparent = _object(runtime, "reparent")
    require(reparent.get("scope") == "Qt-borrowed-device-cross-window-reparent" and
            reparent.get("physical_device_loss_observed") is False and reparent.get("shutdown_failed") is False,
            "unqualified reparent/shutdown claim")
    _integer(reparent, "recapture_delivery_drops", 1, 1)
    require(reparent.get("recapture_completed_before_invalidation") is True, "recapture did not invalidate a completed current payload")
    suspensions = reparent.get("suspensions")
    require(isinstance(suspensions, list) and len(suspensions) == 2, "missing suspend observations")
    for observation, kind in zip(suspensions, ("hidden", "zero-size")):
        require(isinstance(observation, dict) and observation.get("kind") == kind and
                observation.get("observed") is True and observation.get("visible") is (kind == "zero-size"),
                "suspend prerequisite was not observed")
        _integer(observation, "logical_width", 1 if kind == "hidden" else 0, 4096 if kind == "hidden" else 0)
        _integer(observation, "logical_height", 1 if kind == "hidden" else 0, 4096 if kind == "hidden" else 0)
        _integer(observation, "observed_ms", 200, 20000)
        _integer(observation, "rendered_delta", 0, 0)
    stages = reparent.get("stages")
    require(isinstance(stages, list) and len(stages) == 4, "missing reparent stages")
    by_id = {view["id"]: view for view in views}
    previous = None
    survivor_generation = None
    for index, name in enumerate(("initial", "resumed", "detached", "returned")):
        stage = stages[index]
        ids = ["view-" + str(index * 2 + i) for i in range(2)]
        shared = name != "detached"
        require(isinstance(stage, dict) and stage.get("name") == name and stage.get("view_ids") == ids and
                stage.get("shared_device") is shared and stage.get("moving_in_primary_window") is shared and
                stage.get("survivor_device_unchanged") is True, "incorrect Qt device/window ownership transition")
        current = [by_id[ident] for ident in ids]
        for i, view in enumerate(current):
            generation = _integer(view, "device_generation", 1, 2 ** 31 - 1)
            _integer(view, "resource_releases", generation - 1, generation - 1)
            _integer(view, "readback_device_generation", generation, generation)
            capture = _integer(view, "capture_generation", 1, 2 ** 31 - 1)
            require(view["owner_generation"] == 1, "reparent replaced rather than moved the widget")
            if previous:
                require(capture > previous[i]["capture_generation"] and
                        view["readback_frame"] > previous[i]["readback_frame"], "reparent reused stale readback")
            if i == 1:
                if survivor_generation is None:
                    survivor_generation = generation
                require(generation == survivor_generation, "survivor device was recreated")
            elif index >= 2:
                require(generation > previous[i]["device_generation"], "moving scene was not released/reinitialized")
        previous = current
    return ["reparent:hidden-resume", "reparent:zero-size-resume", "reparent:resources-reinitialized",
            "reparent:separate-window-device", "reparent:survivor-stable", "reparent:return-to-original-device",
            "recapture:stale-delivery-dropped"]


def validate_recovery(runtime, views, backend, device):
    recovery = _object(runtime, "recovery")
    require(recovery.get("scope") == "injected-host-failures-and-owned-device-recreation" and
            recovery.get("injection_method") == "host-result-substitution" and
            recovery.get("physical_device_loss_observed") is False, "unqualified recovery claim")
    _integer(recovery, "owned_rhi_creations", 7, 7)
    _integer(recovery, "owned_rhi_destructions", 7, 7)
    faults = recovery.get("faults")
    require(isinstance(faults, list) and len(faults) == 4, "missing/extra injected faults")
    checked = []
    for index, point in enumerate(("device-create", "texture-create", "begin-frame", "submit")):
        fault = faults[index]
        require(isinstance(fault, dict) and fault.get("fault") == point and fault.get("handled") is True and
                fault.get("output_id") == "view-" + str(index), "wrong/unhandled recovery fault")
        failed = _object(fault, "failed_attempt")
        recovered = _object(fault, "recovered_attempt")
        _integer(failed, "owner_generation", index * 2 + 1, index * 2 + 1)
        _integer(recovered, "owner_generation", index * 2 + 2, index * 2 + 2)
        for attempt, expected in ((failed, 0 if point == "device-create" else 1), (recovered, 1)):
            _integer(attempt, "owned_rhi_creations", expected, expected)
            _integer(attempt, "owned_rhi_destructions", expected, expected)
            require(attempt.get("actual_api") == (backend if expected else ""), "recovery changed the requested API")
            if expected:
                _string(attempt, "device_name")
                classification = _string(attempt, "device_class")
                require(classification in ("software", "hardware", "virtual", "unknown") and
                        (device == "any" or classification == device), "wrong recovery device classification")
            else:
                require(attempt.get("device_name") == attempt.get("device_class") == "",
                        "uninitialized failure invented device metadata")
        submitted = point == "submit"
        require(failed.get("frame_started") is submitted and failed.get("readback_completed") is submitted and
                failed.get("image_accepted") is False, "failed attempt accepted pixels or has inconsistent frame state")
        _integer(failed, "actual_submit_result", 0 if submitted else -1, 0 if submitted else -1)
        _integer(failed, "effective_submit_result", 3 if submitted else -1, 3 if submitted else -1)
        require(recovered.get("frame_started") is True and recovered.get("readback_completed") is True and
                recovered.get("image_accepted") is True, "recovered attempt did not render/read back")
        _integer(recovered, "actual_submit_result", 0, 0)
        _integer(recovered, "effective_submit_result", 0, 0)
        require(views[index]["id"] == fault["output_id"] and
                views[index]["owner_generation"] == recovered["owner_generation"], "pixels not bound to recovered owner")
        checked.append("recovery:" + point)
    return checked + ["recovery:owned-devices-destroyed", "recovery:injected-not-physical-loss"]


def validate_lifecycle(runtime, mode, views, backend, device):
    life = _object(runtime, "lifecycle")
    if mode == "offscreen":
        require(life.get("scope") == "owned-QRhi-teardown-recreate", "wrong lifecycle scope")
        require(_integer(life, "owned_rhi_creations", 3, 3) == _integer(life, "owned_rhi_destructions", 3, 3),
                "owned RHI lifetime counts disagree")
        generations = life.get("generations")
        require(isinstance(generations, list) and len(generations) == 3, "missing owned-device generations")
        for i, generation in enumerate(generations, 1):
            require(isinstance(generation, dict) and _integer(generation, "generation", i, i) == i and
                    generation.get("actual_api") == backend and generation.get("readback_completed") is True and
                    generation.get("owner_destroyed") is True, "incomplete owned-device recreation")
            _string(generation, "device_name")
            kind = _string(generation, "device_class")
            require(kind in ("hardware", "software", "virtual", "unknown") and (device == "any" or kind == device),
                    "wrong recreated device classification")
            require(views[i - 1].get("owner_generation") == i, "wrong offscreen owner generation")
        return ["lifecycle:three-owned-devices-created", "lifecycle:three-owned-devices-destroyed"]
    require(life.get("scope") == "widget-consumer-invalidation-and-surface-replacement", "wrong widget lifecycle scope")
    _integer(life, "consumer_invalidations", 1, 1)
    _integer(life, "stale_deliveries_dropped", 1, 1)
    _integer(life, "surface_replacements", 1, 1)
    completed = _integer(life, "gpu_completed_after_invalidation", 0, 1)
    for name in ("gpu_pending_at_invalidation", "gpu_pending_at_destruction"):
        require(type(life.get(name)) is bool, "missing/invalid pending-GPU observation")
    require(life.get("close_request_stage") == "after-readback-recording-before-submission" and
            life["gpu_pending_at_invalidation"] is True and completed == 1 and
            (not life["gpu_pending_at_destruction"] or life["gpu_pending_at_invalidation"]),
            "inconsistent GPU completion/close observations")
    require(life.get("shutdown_failed") is False and life.get("device_loss_injected") is False, "failed/unqualified shutdown")
    require(views[0]["owner_generation"] == 2 and views[1]["owner_generation"] == 1, "replacement surface was not rendered")
    interactions = _object(runtime, "interactions")
    if runtime.get("qpa") == "wayland":
        require(_object(interactions, "target").get("native_popup_seed") is True and
                interactions.get("popup_dispatch") == "native-F12-key-handler",
                "Wayland popup has no observed native compositor input")
    require(interactions.get("dispatch") == "synthetic-Qt-events-with-observed-focus" and
            interactions.get("focused_view") == "view-0" and interactions.get("keyboard_recipient") == "view-0" and
            interactions.get("focus_observed") is True, "unqualified input/focus evidence")
    for name, expected in (("target", 1), ("other_view", 0)):
        view = _object(interactions, name)
        for field in ("mouse_presses", "space_keys", "context_requests", "menu_actions"):
            _integer(view, field, expected, expected)
        require(view.get("popup_observed") is bool(expected), "context popup was not observed on the intended view")
    return ["lifecycle:invalid-consumer-dropped", "lifecycle:surface-replaced", "input:focus-mouse-key-context-menu"]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("report", type=Path, nargs='?')
    parser.add_argument("--cases", type=Path, help='validate logical requirements only; never execute tests')
    parser.add_argument("--stage", choices=tuple('R' + str(i) for i in range(1, 8)))
    parser.add_argument("--configuration", help='exact expanded configuration ID from --cases inventory')
    parser.add_argument("--backend", choices=("vulkan", "metal"))
    parser.add_argument("--device", choices=("software", "hardware", "any"))
    parser.add_argument("--mode", choices=("offscreen", "widget"))
    parser.add_argument("--expected-revision")
    parser.add_argument("--expected-arch")
    parser.add_argument("--expected-dpr", type=float)
    parser.add_argument("--expected-qpa")
    parser.add_argument("--background", choices=("opaque", "transparent"))
    parser.add_argument("--scenario", choices=("standard", "lifecycle", "recovery", "reparent"))
    args = parser.parse_args()
    if args.cases is not None:
        if any(getattr(args, name) is not None for name in ('report', 'backend', 'device', 'mode', 'expected_revision',
                'expected_arch', 'expected_dpr', 'expected_qpa', 'background', 'scenario')):
            parser.error('--cases is mutually exclusive with report/backend evidence arguments')
        if (args.stage is None) != (args.configuration is None):
            parser.error('--stage and --configuration must be supplied together')
        try:
            inventory = validate_case_manifest(args.cases)
            result = (select_case_requirements(inventory, args.stage, args.configuration) if args.stage
                      else case_inventory_summary(inventory))
            print(json.dumps(result, sort_keys=True, indent=2))
        except (OSError, ValueError, TypeError, KeyError) as error:
            parser.exit(1, 'Renderer requirements: FAIL: {}\n'.format(error))
        return
    if args.stage is not None or args.configuration is not None:
        parser.error('--stage/--configuration require --cases')
    if args.report is None or any(getattr(args, name) is None for name in ('backend', 'device', 'mode')):
        parser.error('report, --backend, --device and --mode are required for R0 evidence validation')
    try:
        checks = validate_report(args.report, args.backend, args.device, args.mode,
                                 args.expected_revision, args.expected_arch, args.expected_dpr, args.expected_qpa,
                                 args.background or 'opaque', args.scenario or 'standard')
        print("R0 renderer inventory: PASS ({} pixel/composition checks)".format(len(checks)))
    except (OSError, ValueError, TypeError, KeyError, zlib.error) as error:
        parser.exit(1, "R0 renderer inventory: FAIL: {}\n".format(error))


if __name__ == "__main__":
    main()
