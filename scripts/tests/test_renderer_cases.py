"""CPU renderer source/input contracts and independent fixture oracles, not render proof."""

import base64
import copy
import json
import math
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import validate_renderer_inventory as inventory  # noqa: E402


HEADER_PATH = 'lcviewernoqt/painters/lcpainter.h'
HEADER = '''class LcPainter {
public:
    virtual ~LcPainter() = default;
    virtual void move_to(double x, double y = 0) = 0;
    virtual double scale() = 0;
    virtual void scale(double value) = 0;
    virtual std::vector<std::string> getFontList() const = 0;
};
'''


class PainterSignatureTests(unittest.TestCase):
    def parse(self, declaration):
        return inventory.painter_signatures(
            'class LcPainter { public: virtual ~LcPainter() = default; ' + declaration + ' };')

    def test_all_current_signatures_and_overloads(self):
        signatures = inventory.painter_signatures((inventory.REPO_ROOT / HEADER_PATH).read_text())
        self.assertEqual(60, len(signatures))
        self.assertEqual(58, len({s.split('(')[0].rsplit(' ', 1)[-1] for s in signatures}))
        for signature in ('void clear(double,double,double)', 'void clear(double,double,double,double)',
                          'double scale()', 'void scale(double)', 'unsigned char* data()',
                          'void set_dash(const double*,const int,double,bool)',
                          'std::vector<std::string> getFontList() const',
                          'void addFontsFromPath(const std::vector<std::string>&)'):
            self.assertIn(signature, signatures)

    def test_formatting_comments_and_names_preserve_types(self):
        left = self.parse('virtual unsigned int* example(const std::vector<std::string>& items) const = 0;')
        right = self.parse('''virtual unsigned/* boundary */int *example(
            std :: vector < std :: string > const & renamed) const = 0; // note
        ''')
        self.assertEqual(['unsigned int* example(const std::vector<std::string>&) const'], left)
        self.assertEqual(left, right)

    def test_parameter_names_and_numeric_defaults_are_not_overloads(self):
        self.assertEqual(inventory.painter_signatures(HEADER), inventory.painter_signatures(
            HEADER.replace('double x, double y = 0', 'double renamed, double other = -1.5e+2')))

    def test_nested_templates_pointers_and_references(self):
        self.assertEqual(['std::vector<std::vector<std::string>> list(const char*,LcPainter*const&) const'],
                         self.parse('''virtual std::vector<std::vector<std::string>> list(
                             const char* text, LcPainter* const& painter) const = 0;'''))

    def test_valid_new_declaration_is_not_omitted(self):
        changed = HEADER.replace('virtual double scale()', 'virtual bool enabled() = 0; virtual double scale()')
        self.assertEqual(5, len(inventory.painter_signatures(changed)))
        self.assertIn('bool enabled()', inventory.painter_signatures(changed))

    def test_unsupported_declarations_never_disappear(self):
        for declaration in ('void ignored();', 'int field;', 'LcPainter();',
                            'virtual void nonpure();', 'virtual void f() noexcept = 0;',
                            'virtual void f() & = 0;', '[[nodiscard]] virtual int f() = 0;',
                            'virtual int operator()() = 0;', 'struct Nested { int field; };',
                            '#if 1\nvirtual void conditional() = 0;\n#endif',
                            'virtual void unfinished() = 0', 'private: virtual void hidden() = 0;'):
            with self.subTest(declaration=declaration), self.assertRaises(ValueError):
                self.parse(declaration)

    def test_duplicate_and_return_only_overloads_rejected(self):
        for declaration in ('virtual double f() = 0; virtual double f() = 0;',
                            'virtual double f() = 0; virtual bool f() = 0;',
                            'virtual void f(int x) = 0; virtual void f(int y = 0) = 0;'):
            with self.subTest(declaration=declaration), self.assertRaisesRegex(ValueError, 'duplicate'):
                self.parse(declaration)

    def test_public_api_and_one_default_virtual_destructor_required(self):
        for changed in (HEADER.replace('public:', 'private:'),
                        HEADER.replace('public:', ''),
                        HEADER.replace('virtual ~LcPainter() = default;', ''),
                        HEADER.replace('virtual ~LcPainter() = default;', '~LcPainter() = default;'),
                        HEADER.replace('virtual ~LcPainter() = default;', 'virtual ~LcPainter() = 0;'),
                        HEADER.replace('virtual ~LcPainter() = default;',
                                       'virtual ~LcPainter() = default; virtual ~LcPainter() = default;')):
            with self.subTest(changed=changed), self.assertRaises(ValueError):
                inventory.painter_signatures(changed)

    def test_missing_ambiguous_and_incomplete_class_rejected(self):
        for changed in ('class LcPainter;', HEADER + HEADER, HEADER[:-3],
                        HEADER.replace('class LcPainter', 'class LcPainter final'),
                        'const char* fake = R"(class LcPainter { public: virtual void f() = 0; };)";'):
            with self.subTest(changed=changed), self.assertRaises(ValueError):
                inventory.painter_signatures(changed)

    def test_empty_malformed_or_unnamed_parameters_rejected(self):
        for params in (',', 'double x,', ',double x', 'double x,,double y', 'double', 'void',
                       'const double', 'void (*callback)()', 'double values[2]',
                       'std::vector<std::string value', 'std::vector<> value',
                       'std::vector<,std::string> value', 'auto value',
                       'double x, int x', 'double x = 0, double y',
                       'double x = make_value()', 'double x = 1+2'):
            with self.subTest(params=params), self.assertRaises(ValueError):
                self.parse('virtual void f(' + params + ') = 0;')

    def test_empty_oversized_and_unterminated_input_rejected(self):
        for text in (None, '', ' ' * (128 * 1024 + 1), '/*' + HEADER, '*/' + HEADER,
                     HEADER + '/*', HEADER + '"unterminated',
                     'class LcPainter { public: virtual ~LcPainter() = default; };'):
            with self.subTest(text_type=type(text)), self.assertRaises(ValueError):
                inventory.painter_signatures(text)
        with self.assertRaises(ValueError):
            self.parse('virtual void f(' + 'std::vector<' * 10 + 'int' + '>' * 10 + ' x) = 0;')
        with self.assertRaises(ValueError):
            self.parse(' '.join('virtual void f{}() = 0;'.format(i) for i in range(129)))

    def test_comments_literals_and_line_splices_keep_token_boundaries(self):
        text = '// virtual void fake() = 0;\n' + HEADER.replace('move_to', 'move_\\\nto')
        self.assertEqual(inventory.painter_signatures(HEADER), inventory.painter_signatures(text))
        self.assertEqual('unsigned int', inventory._cpp_without_comments('unsigned/* note */int'))

    def test_numeric_digit_separators_are_not_character_literals(self):
        self.assertEqual("int count = 1'000; char x =  ;", inventory._cpp_without_comments("int count = 1'000; char x = 'x';"))
        # The bounded API grammar deliberately requires review for default
        # formats beyond decimal literals/bools/nullptr, rather than dropping it.
        with self.assertRaisesRegex(ValueError, 'unsupported painter default'):
            self.parse("virtual void f(int count = 1'000) = 0;")


class SourceRegisterTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        for source in inventory._CONSUMER_ROLES:
            self.write(source, '// test consumer\n')
        for source in ('lcviewernoqt/drawitems/line.cpp', 'lcviewernoqt/drawitems/line.h',
                       'lcviewernoqt/drawables/grid.cpp', 'lcviewernoqt/drawables/grid.h'):
            self.write(source, '// test drawable\n')
        self.write(HEADER_PATH, HEADER)
        self.register = {
            'painter_header': HEADER_PATH,
            'header_sha256': inventory.sha256((self.root / HEADER_PATH).read_bytes()),
            'methods': inventory.painter_signatures(HEADER),
            'consumers': [dict(id='consumer-' + str(index), source=source, kind=kind)
                          for index, (source, kind) in enumerate(sorted(inventory.consumer_sources(self.root).items()))],
        }

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding='utf-8')
        return path

    def validate(self):
        return inventory.validate_source_register(self.register, self.root)

    def refresh_header_digest(self):
        self.register['header_sha256'] = inventory.sha256((self.root / HEADER_PATH).read_bytes())

    def test_valid_register_and_reordered_records(self):
        self.register['methods'].reverse()
        self.register['consumers'].reverse()
        methods, consumers = self.validate()
        self.assertEqual(set(inventory.painter_signatures(HEADER)), methods)
        self.assertEqual(len(self.register['consumers']), len(consumers))

    def test_header_hash_and_parse_use_one_byte_snapshot(self):
        reads = []
        expected = (self.root / HEADER_PATH).resolve()
        original_bytes, original_text = Path.read_bytes, Path.read_text

        def read(path):
            if path == expected:
                reads.append(path)
                self.assertEqual(1, len(reads), 'Digest and parser reread different header snapshots')
            return original_bytes(path)

        def read_text(path, *args, **kwargs):
            self.assertNotEqual(expected, path.resolve(), 'Parser reread the hashed header as text')
            return original_text(path, *args, **kwargs)

        with patch.object(Path, 'read_bytes', read), patch.object(Path, 'read_text', read_text):
            self.validate()
        self.assertEqual(1, len(reads))

    def test_actual_repository_register_includes_public_event_and_factory_headers(self):
        header = inventory.REPO_ROOT / HEADER_PATH
        roles = inventory.consumer_sources()
        register = dict(painter_header=HEADER_PATH, header_sha256=inventory.sha256(header.read_bytes()),
                        methods=inventory.painter_signatures(header.read_text()),
                        consumers=[dict(id='source-' + str(i), source=source, kind=kind)
                                   for i, (source, kind) in enumerate(sorted(roles.items()))])
        methods, consumers = inventory.validate_source_register(register)
        self.assertEqual(60, len(methods))
        self.assertEqual(len(roles), len(consumers))
        for path, kind in (('lcviewernoqt/events/drawevent.h', 'canvas'),
                           ('lcviewernoqt/painters/createpainter.h', 'canvas'),
                           ('lcUI/lcadviewer.h', 'widget'),
                           ('lcviewernoqt/drawitems/lcimage.h', 'drawable'),
                           ('lcviewernoqt/painters/lccairopainter.tcc', 'cairo'),
                           ('luacmdinterface/main.cpp', 'cli')):
            self.assertEqual(kind, roles[path])

    def test_missing_and_unknown_projection_fields(self):
        original = copy.deepcopy(self.register)
        for value in (None, [], '', 1):
            with self.subTest(value=value), self.assertRaises(ValueError):
                inventory.validate_source_register(value, self.root)
        for key in original:
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.register = {k: v for k, v in original.items() if k != key}
                self.validate()
        self.register = dict(original, executed=True)
        with self.assertRaises(ValueError):
            self.validate()

    def test_default_drift_requires_new_header_digest(self):
        changed = HEADER.replace('double y = 0', 'double y = 1')
        self.write(HEADER_PATH, changed)
        self.assertEqual(self.register['methods'], inventory.painter_signatures(changed))
        with self.assertRaisesRegex(ValueError, 'stale painter header/defaults'):
            self.validate()
        self.refresh_header_digest()
        self.validate()

    def test_api_drift_even_with_fresh_header_digest(self):
        for changed in (HEADER.replace('double scale()', 'bool scale()'),
                        HEADER.replace('double x,', 'int x,'),
                        HEADER.replace('getFontList() const', 'getFontList()'),
                        HEADER.replace('move_to', 'line_to'),
                        HEADER.replace('virtual double scale() = 0;', ''),
                        HEADER.replace('virtual double scale()', 'virtual void added() = 0; virtual double scale()')):
            with self.subTest(changed=changed), self.assertRaisesRegex(ValueError, 'API drift'):
                self.write(HEADER_PATH, changed)
                self.refresh_header_digest()
                self.validate()

    def test_malformed_duplicate_or_orphan_signatures(self):
        original = list(self.register['methods'])
        for methods in (None, [], [None], [{}], [' '], original + original[:1],
                        original[:-1], original + ['void unknown()']):
            with self.subTest(methods=methods), self.assertRaises(ValueError):
                self.register['methods'] = methods
                self.validate()

    def test_wrong_header_location_or_digest(self):
        for path in ('lcUI/lcadviewer.cpp', '../outside.h', '/tmp/header.h'):
            with self.subTest(path=path), self.assertRaises(ValueError):
                self.register['painter_header'] = path
                self.validate()
        self.register['painter_header'] = HEADER_PATH
        for digest in ('a' * 64, 'A' * 64, 'short', None, []):
            with self.subTest(digest=digest), self.assertRaises(ValueError):
                self.register['header_sha256'] = digest
                self.validate()

    def test_new_nested_drawable_source_and_header_require_mapping(self):
        for suffix in ('.cpp', '.h', '.hpp', '.CPP', '.HXX'):
            added = self.write('lcviewernoqt/drawitems/nested/new' + suffix, '// no direct painter token\n')
            with self.subTest(suffix=suffix), self.assertRaisesRegex(ValueError, 'source drift'):
                self.validate()
            added.unlink()

    def test_deleted_drawable_or_required_host_is_not_ignored(self):
        for source in ('lcviewernoqt/drawitems/line.h', 'lcUI/main.cpp'):
            path = self.root / source
            text = path.read_text()
            path.unlink()
            with self.subTest(source=source), self.assertRaisesRegex(ValueError, 'missing/outside'):
                self.validate()
            self.write(source, text)

    def test_unknown_direct_callers_require_review_in_all_production_roots(self):
        for folder in ('lcUI', 'lcviewernoqt', 'luacmdinterface', 'lcscripting', 'lcadluascript', 'lcadpythonscript'):
            for token in ('LcPainter* painter;', 'createOpenGLPainter(nullptr, 1, 1);',
                          'createCairoImagePainter(nullptr, 1, 1);'):
                path = self.write(folder + '/new.hpp', '#if OTHER_PLATFORM\n' + token + '\n#endif\n')
                with self.subTest(folder=folder, token=token), self.assertRaisesRegex(ValueError, 'unclassified painter consumer'):
                    self.validate()
                path.unlink()

    def test_literals_and_comments_do_not_invent_callers_or_hide_real_ones(self):
        for content in ('// LcPainter* fake;\n', '/* createOpenGLPainter(); */\n',
                        'const char* text = "https://example/LcPainter";\n',
                        'const char* text = R"tag( // LcPainter /* )tag";\n',
                        "int count = 1'000; const char* text = u8\"LcPainter\";\n",
                        '// continued \\\nLcPainter* fake;\n'):
            path = self.write('lcUI/noncaller.cpp', content)
            self.validate()
            path.unlink()
        self.write('lcUI/new.cpp', 'const char* url = "http://example"; LcPainter* actual;\n')
        with self.assertRaisesRegex(ValueError, 'unclassified painter consumer'):
            self.validate()

    def test_backend_and_new_test_implementations_are_not_production_consumers(self):
        for source in ('lcviewernoqt/painters/opengl/implementation.cpp',
                       'lcrenderer/rhi/implementation.cpp', 'unittest/rendering/new_contract.cpp'):
            self.write(source, 'LcPainter* implementation;\n')
        self.validate()

    def test_consumer_roles_cannot_be_relabeled_to_weaken_coverage(self):
        for consumer in self.register['consumers']:
            original = consumer['kind']
            consumer['kind'] = 'build' if original != 'build' else 'drawable'
            with self.subTest(source=consumer['source']), self.assertRaisesRegex(ValueError, 'false consumer role'):
                self.validate()
            consumer['kind'] = original
        drawable = next(c for c in self.register['consumers'] if c['kind'] == 'drawable')
        for kind in ('cli', 'cairo', 'widget', 'scripting', 'legacy-reference', 'unknown'):
            with self.subTest(kind=kind), self.assertRaisesRegex(ValueError, 'false consumer role'):
                drawable['kind'] = kind
                self.validate()

    def test_duplicate_consumer_id_or_source(self):
        duplicate = copy.deepcopy(self.register['consumers'][0])
        self.register['consumers'].append(duplicate)
        with self.assertRaisesRegex(ValueError, 'consumer ID'):
            self.validate()
        duplicate['id'] = 'another-id'
        with self.assertRaisesRegex(ValueError, 'consumer source'):
            self.validate()

    def test_missing_or_unknown_consumer(self):
        consumer = self.register['consumers'].pop()
        with self.assertRaisesRegex(ValueError, 'source drift'):
            self.validate()
        self.register['consumers'].append(consumer)
        self.write('unrelated.cpp', '// not a consumer\n')
        self.register['consumers'].append(dict(id='unknown', source='unrelated.cpp', kind='build'))
        with self.assertRaisesRegex(ValueError, 'unknown consumer source'):
            self.validate()

    def test_malformed_inventory_and_consumer_fields(self):
        original = copy.deepcopy(self.register['consumers'])
        for value in (None, [], [{}], [None], original * 11):
            with self.subTest(value_type=type(value)), self.assertRaises(ValueError):
                self.register['consumers'] = value
                self.validate()
        self.register['consumers'] = original
        for key in ('id', 'source', 'kind'):
            for value in (None, [], {}, '', ' '):
                original_value = self.register['consumers'][0][key]
                with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                    self.register['consumers'][0][key] = value
                    self.validate()
                self.register['consumers'][0][key] = original_value
        self.register['consumers'][0]['executed'] = True
        with self.assertRaises(ValueError):
            self.validate()

    def test_unsafe_and_external_source_paths(self):
        consumer = self.register['consumers'][0]
        for source in ('../outside.cpp', '/tmp/outside.cpp', 'C:/outside.cpp',
                       'lcUI\\main.cpp', 'lcUI/../lcUI/main.cpp'):
            with self.subTest(source=source), self.assertRaisesRegex(ValueError, 'unsafe'):
                consumer['source'] = source
                self.validate()

    def test_symlink_escape_and_source_size_are_rejected(self):
        path = self.root / 'lcUI/main.cpp'
        original = path.read_bytes()
        path.unlink()
        try:
            path.symlink_to(inventory.REPO_ROOT / HEADER_PATH)
        except OSError:
            # Windows without symlink privilege still exercises the resolved-path
            # boundary; native POSIX runs exercise the real filesystem link.
            path.write_bytes(original)
            real_resolve = Path.resolve
            with patch.object(Path, 'resolve', lambda p: inventory.REPO_ROOT / HEADER_PATH
                              if p == path else real_resolve(p)), self.assertRaisesRegex(ValueError, 'missing/outside'):
                self.validate()
        else:
            with self.assertRaisesRegex(ValueError, 'missing/outside'):
                self.validate()
            path.unlink()
        path.write_bytes(b' ' * (8 * 1024 * 1024 + 1))
        with self.assertRaisesRegex(ValueError, 'oversized'):
            self.validate()

    def test_manifest_reuses_the_same_source_register_validation(self):
        self.register['methods'].append('void absent()')
        manifest = dict(self.register, schema_version=2, evidence_kind='renderer-case-requirements',
                        fixtures=[], configurations=[], oracles=[], cases=[], workloads=[], notes=['M1 test'])
        path = self.write('manifest.json', json.dumps(manifest))
        with patch.object(inventory, 'validate_source_register', wraps=inventory.validate_source_register) as validate:
            with self.assertRaisesRegex(ValueError, 'API drift'):
                inventory.validate_case_manifest(path, self.root)
            validate.assert_called_once()

    def test_existing_report_cli_still_requires_report_api_device_and_mode(self):
        tool = inventory.REPO_ROOT / 'scripts/validate_renderer_inventory.py'
        help_result = subprocess.run([sys.executable, '-B', str(tool), '--help'], text=True, capture_output=True)
        self.assertEqual(0, help_result.returncode, help_result.stderr)
        for flag in ('report', '--backend', '--device', '--mode'):
            self.assertIn(flag, help_result.stdout)
        missing = subprocess.run([sys.executable, '-B', str(tool)], text=True, capture_output=True)
        self.assertEqual(2, missing.returncode)


class TypedInputTests(unittest.TestCase):
    def value(self, kind, value, unit='none', expected=False):
        inventory._typed_value(dict(type=kind, unit=unit, value=value), expected)

    def test_valid_bounded_types_and_contexts(self):
        for kind, value, unit in (
                ('number', 1.5, 'model'), ('integer', 42, 'count'), ('boolean', True, 'none'),
                ('text', 'Hé🙂', 'none'), ('strings', ['a', 'b'], 'none'),
                ('numbers', [1, 2], 'radian'), ('integers', [0x1f642], 'codepoint'),
                ('vec2', [1, 2], 'model'), ('points2', [[0, 0], [3, 4]], 'model'),
                ('matrix3', [1, 0, 2, 0, 1, 3, 0, 0, 1], 'none'),
                ('bytes', [0, 128, 255], '8bit-channel'), ('asset', 'abeezee', 'none'),
                ('nonfinite', 'NaN', 'ratio')):
            with self.subTest(kind=kind):
                self.value(kind, value, unit)
        self.value('configuration', 'backend', expected=True)

    def test_wrong_scalar_types_nonfinite_and_huge_numbers(self):
        for kind, unit, values in (
                ('number', 'model', [True, None, '1', math.nan, math.inf, -math.inf, 10 ** 400]),
                ('integer', 'count', [True, 1.0, 2 ** 53, -(2 ** 53)]),
                ('boolean', 'none', [0, 1, 'true']),
                ('text', 'none', ['', 'x' * 4097, '\0', '\ud800'])):
            for value in values:
                if kind == 'text' and value == '':
                    self.value(kind, value, unit)  # Empty text is meaningful input.
                    continue
                with self.subTest(kind=kind, value=repr(value)[:40]), self.assertRaises(ValueError):
                    self.value(kind, value, unit)

    def test_vectors_arrays_matrices_and_bytes_are_bounded(self):
        for kind, unit, values in (
                ('vec2', 'model', [[], [1], [1, 2, 3], [1, True]]),
                ('points2', 'model', [[[1]], [[1, math.inf]], [[0, 0]] * 513]),
                ('matrix3', 'none', [[1] * 8, [1] * 9, [1, 0, 0, 0, 1, 0, 0, 0, True]]),
                ('bytes', 'byte', [[-1], [256], [True], [1.0], [0] * 4097]),
                ('strings', 'none', [[1], ['x'] * 129]),
                ('integers', 'codepoint', [[-1], [0x110000], [0xd800], [0xdfff]])):
            for value in values:
                with self.subTest(kind=kind, length=len(value)), self.assertRaises(ValueError):
                    self.value(kind, value, unit)

    def test_context_units_and_unknown_fields(self):
        for kind, value, unit, expected in (
                ('configuration', 'backend', 'none', False),
                ('configuration', 'unknown', 'none', True),
                ('nonfinite', 'NaN', 'ratio', True), ('nonfinite', 'nan', 'ratio', False),
                ('asset', 'abeezee', 'none', True), ('asset', '../font', 'none', False),
                ('vec2', [0, 1], 'radian', False), ('bytes', [1], 'model', False),
                ('unknown', 1, 'none', False)):
            with self.subTest(kind=kind, expected=expected), self.assertRaises(ValueError):
                self.value(kind, value, unit, expected)
        for record in ({'type': 'text', 'value': 'x'},
                       {'type': 'text', 'unit': 'none', 'value': 'x', 'executed': True}, None):
            with self.assertRaises(ValueError):
                inventory._typed_value(record)

    def test_json_snapshot_rejects_duplicates_nonfinite_and_invalid_encoding(self):
        for raw in (b'{"x":1,"x":2}', b'{"x":NaN}', b'{"x":Infinity}', b'\xff',
                    b'{}' + b' ' * (2 * 1024 * 1024)):
            with self.subTest(raw=raw[:30]), self.assertRaises(ValueError):
                inventory._json_bytes(raw)


class CaseInputTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Real repository positive first. Isolated mutations mock only M1's
        # already-tested source projection, never M2 fixture/input validation.
        cls.real = inventory.validate_case_inputs()
        cls.base = cls.real['manifest']
        cls.corpus_path = 'unittest/rendering/fixtures/renderer_contracts.json'
        cls.corpus_base = inventory.read_json(inventory.REPO_ROOT / cls.corpus_path)

    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.manifest, self.corpus = copy.deepcopy(self.base), copy.deepcopy(self.corpus_base)
        for fixture in self.base['fixtures']:
            for name in (fixture['path'], fixture['notice']):
                path = self.root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(inventory.REPO_ROOT / name, path)

    def validate(self, rehash=True):
        path = self.root / self.corpus_path
        path.write_bytes(json.dumps(self.corpus).encode())
        if rehash:
            self.manifest['fixtures'][0]['sha256'] = inventory.sha256(path.read_bytes())
        manifest = self.root / 'manifest.json'
        manifest.write_bytes(json.dumps(self.manifest).encode())
        with patch.object(inventory, 'validate_source_register', return_value=(
                self.real['methods'], self.real['consumers'])) as source:
            result = inventory.validate_case_inputs(manifest, self.root)
            source.assert_called_once()
            return result

    def case(self, ident):
        return next(c for c in self.manifest['cases'] if c['id'] == ident)

    def variant(self, ident='line', name='ordinary'):
        return self.corpus['datasets'][ident][name]

    def test_real_repository_complete_input_bindings_not_execution(self):
        self.assertEqual(60, len(self.real['methods']))
        self.assertEqual(75, len(self.real['consumers']))
        self.assertEqual({'F' + str(i) for i in range(1, 13)},
                         {f for c in self.base['cases'] for f in c['features']})
        self.assertEqual(56, len(self.base['cases']))
        self.assertEqual(223, len(self.real['requirements']))
        self.assertEqual(6, len(self.real['fixtures']))
        self.assertNotIn('configurations', self.real)  # M3 is not qualified here.
        for required in self.real['requirements']:
            self.assertIn(required['binding'], ('proposed-gtest', 'proposed-runner-check'))
            self.assertNotIn('executed', required)
        self.assertEqual(len(self.real['requirements']), len(self.validate()['requirements']))

    def test_hash_notice_kind_license_provenance_and_path_mutations(self):
        mutations = [('sha256', '0' * 64), ('notice_sha256', '0' * 64), ('kind', 'executable'),
                     ('license', 'BSD-3-Clause'), ('provenance', ''), ('path', '../outside'),
                     ('path', '/tmp/outside'), ('path', 'C:/outside'), ('path', 'a\\b')]
        for key, value in mutations:
            with self.subTest(key=key, value=value):
                fixture = self.manifest['fixtures'][1]
                old = fixture[key]
                fixture[key] = value
                with self.assertRaises(ValueError):
                    self.validate()
                fixture[key] = old
        self.manifest['fixtures'][1]['license'] = 'GPL-3.0-or-later'
        with self.assertRaisesRegex(ValueError, 'font license'):
            self.validate()

    def test_duplicate_fixture_ids_paths_and_missing_font_notice(self):
        original = copy.deepcopy(self.manifest)
        for key in ('id', 'path'):
            self.manifest = copy.deepcopy(original)
            self.manifest['fixtures'][1][key] = self.manifest['fixtures'][0][key]
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate()
        self.manifest = original
        notice = self.root / self.manifest['fixtures'][1]['notice']
        notice.write_bytes(b'Not the OFL license')
        self.manifest['fixtures'][1]['notice_sha256'] = inventory.sha256(notice.read_bytes())
        with self.assertRaisesRegex(ValueError, 'font license'):
            self.validate()

    def test_contract_hash_and_parse_use_one_payload_snapshot(self):
        data = (self.root / self.corpus_path).read_bytes()
        real_read = inventory._bounded_bytes
        reads = []
        def read(path, limit):
            if path == (self.root / self.corpus_path).resolve():
                reads.append(path)
                return data if len(reads) == 1 else b'{}'
            return real_read(path, limit)
        with patch.object(inventory, '_bounded_bytes', side_effect=read):
            inventory._case_fixtures(self.base['fixtures'], self.root, self.real['oracles'])
        self.assertEqual(1, len(reads))

    def test_changed_fixture_bytes_rejected(self):
        path = self.root / self.manifest['fixtures'][2]['path']
        path.write_bytes(path.read_bytes() + b' ')
        with self.assertRaisesRegex(ValueError, 'stale fixture'):
            self.validate()

    def test_script_sources_are_inert_and_format_checked(self):
        self.assertEqual('script-source', self.manifest['fixtures'][-1]['kind'])
        with patch('builtins.exec', side_effect=AssertionError('script executed')):
            self.validate()
        fixture = self.manifest['fixtures'][-1]
        fixture['path'] = self.manifest['fixtures'][2]['path']
        with self.assertRaises(ValueError):
            self.validate()

    def test_missing_empty_or_limit_and_orphan_variants(self):
        for name in ('ordinary', 'empty', 'limit'):
            saved = self.corpus['datasets']['line'].pop(name)
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.validate()
            self.corpus['datasets']['line'][name] = saved
        self.corpus['datasets']['line']['extra'] = copy.deepcopy(self.variant())
        with self.assertRaisesRegex(ValueError, 'orphan'):
            self.validate()

    def test_empty_operands_assertions_duplicate_observations_and_units(self):
        original = copy.deepcopy(self.variant())
        for key in ('inputs', 'assertions'):
            self.corpus['datasets']['line']['ordinary'] = copy.deepcopy(original)
            self.variant()[key] = {} if key == 'inputs' else []
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate()
        self.corpus['datasets']['line']['ordinary'] = copy.deepcopy(original)
        self.variant()['assertions'].append(copy.deepcopy(original['assertions'][0]))
        with self.assertRaisesRegex(ValueError, 'duplicate observed'):
            self.validate()
        self.variant()['assertions'] = copy.deepcopy(original['assertions'])
        self.variant()['assertions'][0]['expected']['unit'] = 'radian'
        with self.assertRaisesRegex(ValueError, 'unit mismatch'):
            self.validate()

    def test_unknown_or_mismatched_oracle_and_performance_not_geometry_error(self):
        assertion = self.variant()['assertions'][0]
        for name in ('unknown', 'contracts', 'interaction-budget'):
            assertion['oracle'] = name
            if name == 'interaction-budget':
                assertion['expected'] = dict(type='number', unit='ratio', value=1)
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.validate()

    def test_oracle_limits_units_decisions_and_orphans(self):
        original = copy.deepcopy(self.manifest['oracles'])
        for key, value in (('limit', True), ('limit', math.nan), ('limit', 10 ** 400),
                           ('limit', -1), ('limit', 1), ('unit', 'model'), ('decision', 'D7')):
            self.manifest['oracles'] = copy.deepcopy(original)
            self.manifest['oracles'][0][key] = value
            with self.subTest(key=key, value=repr(value)[:20]), self.assertRaises(ValueError):
                self.validate()
        self.manifest['oracles'] = original + [dict(original[0], id='orphan')]
        with self.assertRaisesRegex(ValueError, 'unreferenced oracle'):
            self.validate()

    def test_missing_dataset_or_unknown_asset_references(self):
        original = copy.deepcopy(self.manifest)
        for key, value in (('dataset', 'absent'), ('fixture', 'abeezee'), ('assets', ['unknown'])):
            self.manifest = copy.deepcopy(original)
            self.case('line')[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate()
        self.manifest = original
        self.variant()['inputs']['font'] = dict(type='asset', unit='none', value='abeezee')
        with self.assertRaisesRegex(ValueError, 'undeclared'):
            self.validate()

    def test_diagnostic_reason_nonfinite_and_success_are_consistent(self):
        original = copy.deepcopy(self.case('line'))
        self.case('line')['variants'][2]['reason'] = 'different-reason'
        with self.assertRaisesRegex(ValueError, 'diagnostic assertion'):
            self.validate()
        self.manifest['cases'][self.manifest['cases'].index(self.case('line'))] = original
        self.variant()['inputs']['scale'] = dict(type='nonfinite', unit='ratio', value='NaN')
        with self.assertRaisesRegex(ValueError, 'successful case'):
            self.validate()

    def test_bindings_must_be_proposed_unique_and_not_spike_or_disabled(self):
        original = copy.deepcopy(self.case('line'))
        for name in ('DISABLED_Suite.Test', 'R0Spike.Test', 'Suite.*',
                     self.case('point')['variants'][0]['test']):
            self.case('line')['variants'][0]['test'] = name
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.validate()
        self.manifest['cases'][self.manifest['cases'].index(self.case('line'))] = original
        self.case('line')['binding'] = 'executed-gtest'
        with self.assertRaisesRegex(ValueError, 'must be proposed'):
            self.validate()

    def test_package_smoke_never_requires_shipped_gtests(self):
        self.case('line')['configurations'] = ['snap']
        # Use a real declared package ID, rather than pass on an unknown reference.
        package = next(c['id'] for c in self.manifest['configurations'] if c['artifact'] == 'snap')
        self.case('line')['configurations'] = [package]
        with self.assertRaisesRegex(ValueError, 'shipped GTest'):
            self.validate()

    def test_case_coverage_and_orphan_datasets_fail_closed(self):
        self.corpus['datasets']['orphan'] = copy.deepcopy(self.corpus['datasets']['line'])
        with self.assertRaisesRegex(ValueError, 'orphan input dataset'):
            self.validate()
        del self.corpus['datasets']['orphan']
        for case in self.manifest['cases']:
            case['features'] = [f for f in case['features'] if f != 'F12'] or ['F1']
        with self.assertRaisesRegex(ValueError, 'unmapped'):
            self.validate()

    def test_workload_sample_bounds_case_binding_and_diagnostic_variants(self):
        original = copy.deepcopy(self.manifest['workloads'])
        for key, value in (('samples', 0), ('warmups', True), ('case', 'absent'),
                           ('dataset', 'line'), ('variants', ['limit']), ('oracles', ['absent'])):
            self.manifest['workloads'] = copy.deepcopy(original)
            self.manifest['workloads'][0][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate()

    def test_performance_budget_and_hardware_requirements_cannot_be_weakened(self):
        self.manifest['workloads'][0]['oracles'] = ['contracts']
        with self.assertRaisesRegex(ValueError, 'performance/idle budget'):
            self.validate()
        self.manifest['workloads'] = copy.deepcopy(self.base['workloads'])
        self.case('workload-mixed')['devices'] = ['software']
        with self.assertRaisesRegex(ValueError, 'software cannot qualify'):
            self.validate()

    def test_case_schema_configuration_ids_and_proposed_stage_are_strict(self):
        original = copy.deepcopy(self.manifest)
        for key, value in (('stage', 'R0'), ('features', ['F13']), ('methods', ['void unknown()']),
                           ('consumers', ['unknown']), ('profiles', ['unknown']),
                           ('modes', ['headless']), ('devices', ['unknown']), ('decisions', ['D7'])):
            self.manifest = copy.deepcopy(original)
            self.case('line')[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate()
        self.manifest = copy.deepcopy(original)
        self.manifest['cases'].append(copy.deepcopy(self.case('line')))
        with self.assertRaisesRegex(ValueError, 'duplicate case ID'):
            self.validate()
        self.manifest = original
        self.manifest['configurations'].append(copy.deepcopy(self.manifest['configurations'][0]))
        with self.assertRaisesRegex(ValueError, 'duplicate configuration'):
            self.validate()

    def test_script_payload_requires_valid_utf8_and_allowed_suffix(self):
        fixture = self.manifest['fixtures'][-1]
        path = self.root / fixture['path']
        path.write_bytes(b'\xff')
        fixture['sha256'] = inventory.sha256(path.read_bytes())
        with self.assertRaises(UnicodeDecodeError):
            self.validate()
        new = path.with_suffix('.txt')
        new.write_bytes(b'inert fixture')
        fixture['path'] = new.relative_to(self.root).as_posix()
        fixture['sha256'] = inventory.sha256(new.read_bytes())
        with self.assertRaisesRegex(ValueError, 'script fixture format'):
            self.validate()

    def test_full_validator_uses_m2_once_before_draft_selection(self):
        with patch.object(inventory, '_case_fixtures', wraps=inventory._case_fixtures) as fixtures, \
                patch.object(inventory, 'case_configurations', side_effect=ValueError('selection boundary')):
            with self.assertRaisesRegex(ValueError, 'selection boundary'):
                inventory.validate_case_manifest()
        fixtures.assert_called_once()


class CorpusOracleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.inputs = inventory.validate_case_inputs()
        cls.datasets = cls.inputs['datasets']['contracts']

    def body(self, ident, variant='ordinary'):
        return self.datasets[ident][variant]

    def operands(self, ident, variant='ordinary'):
        return {key: value['value'] for key, value in self.body(ident, variant)['inputs'].items()}

    def expected(self, ident, observed, variant='ordinary'):
        return next(a['expected']['value'] for a in self.body(ident, variant)['assertions']
                    if a['observed'] == observed)

    def test_line_and_closed_polyline_lengths(self):
        for ident, variant, field in (('line', 'ordinary', 'length'),
                                      ('path-state', 'ordinary', 'closed-length'),
                                      ('lwpolyline', 'closed', 'length')):
            values = self.operands(ident, variant)
            points = values['points']
            if values.get('closed'):
                points = points + points[:1]
            length = sum(math.hypot(b[0] - a[0], b[1] - a[1]) for a, b in zip(points, points[1:]))
            self.assertAlmostEqual(length, self.expected(ident, field, variant))

    def test_arcs_ellipses_circle_and_bulges(self):
        for variant in ('ordinary', 'clockwise'):
            values = self.operands('arc', variant)
            end = values['angles'][1]
            x, y = values['center']
            result = [x + values['radius'] * math.cos(end), y + values['radius'] * math.sin(end)]
            for got, expected in zip(result, self.expected('arc', 'end', variant)):
                self.assertAlmostEqual(got, expected)
        values = self.operands('arc')
        self.assertAlmostEqual(values['radius'] * abs(values['angles'][1] - values['angles'][0]),
                               self.expected('arc', 'length'))
        values = self.operands('circle')
        self.assertAlmostEqual(math.pi * values['radius'] ** 2, self.expected('circle', 'area'))
        values = self.operands('ellipse')
        for parameter, observed in zip(values['angles'], ('start', 'end')):
            x, y = values['radii'][0] * math.cos(parameter), values['radii'][1] * math.sin(parameter)
            angle = values['rotation']
            result = [x * math.cos(angle) - y * math.sin(angle), x * math.sin(angle) + y * math.cos(angle)]
            for got, expected in zip(result, self.expected('ellipse', observed)):
                self.assertAlmostEqual(got, expected)
        for variant in ('ordinary', 'negative-bulge'):
            values = self.operands('lwpolyline', variant)
            a, b = values['points']
            chord = math.hypot(b[0] - a[0], b[1] - a[1])
            bulge = values['bulges'][0]
            self.assertAlmostEqual(chord * (1 + bulge * bulge) / (4 * abs(bulge)),
                                   self.expected('lwpolyline', 'bulge-radius', variant))
            self.assertAlmostEqual(abs(4 * math.atan(bulge)),
                                   self.expected('lwpolyline', 'absolute-sweep', variant))

    def test_polygon_holes_concavity_and_reversed_winding(self):
        def area(points):
            return sum(a[0] * b[1] - b[0] * a[1] for a, b in zip(points, points[1:] + points[:1])) / 2
        for variant in ('ordinary', 'reversed-hole', 'concave'):
            values = self.operands('hatch-fill', variant)
            vertices = values['vertices']
            starts = values.get('contour-starts', [0]) + [len(vertices)]
            areas = [abs(area(vertices[a:b])) for a, b in zip(starts, starts[1:])]
            self.assertAlmostEqual(areas[0] - sum(areas[1:]), self.expected('hatch-fill', 'area', variant))
        # The ordinary points are strictly within the outer region and hole;
        # this independent axis-aligned membership check is not tessellation.
        values = self.operands('hatch-fill')
        membership = ['inside' if 0 < x < 4 and 0 < y < 4 and not (1 < x < 3 and 1 < y < 3)
                      else 'outside' for x, y in values['sample-points']]
        self.assertEqual(membership, self.expected('hatch-fill', 'membership'))

    def test_bezier_midpoints_endpoints_and_periodic_tangent_seams(self):
        def at(points, t):
            # De Casteljau is independent of the future renderer subdivision.
            while len(points) > 1:
                points = [[(1 - t) * a[d] + t * b[d] for d in (0, 1)]
                          for a, b in zip(points, points[1:])]
            return points[0]
        values = self.operands('spline')
        spans, offset = [], 0
        for count in values['span-control-counts']:
            spans.append(values['control-points'][offset:offset + count])
            offset += count
        self.assertEqual([at(points, 1) for points in spans], self.expected('spline', 'endpoints'))
        self.assertEqual(at(spans[1], .5), self.expected('spline', 'quadratic-midpoint'))
        self.assertEqual(at(spans[2], .5), self.expected('spline', 'cubic-midpoint'))
        values = self.operands('spline', 'periodic')
        points = values['control-points']
        spans = [points[n:n + 4] for n in range(0, len(points), 4)]
        for left, right in zip(spans, spans[1:] + spans[:1]):
            self.assertEqual(left[-1], right[0])
            for d in (0, 1):
                self.assertAlmostEqual(3 * (left[-1][d] - left[-2][d]), 3 * (right[1][d] - right[0][d]))
        self.assertEqual([0, 0], self.expected('spline', 'seam-tangent-gap', 'periodic'))
        # These analytic samples do NOT establish a full display-error bound.

    def test_transform_inverse_nested_insert_and_large_origin(self):
        def transform(m, point, translate=True):
            x, y = point
            return [m[0] * x + m[1] * y + (m[2] if translate else 0),
                    m[3] * x + m[4] * y + (m[5] if translate else 0)]
        values = self.operands('transform')
        matrix = values['transform']
        device = transform(matrix, values['point'])
        self.assertEqual(device, self.expected('transform', 'device-point'))
        self.assertEqual(transform(matrix, values['distance'], False), self.expected('transform', 'device-distance'))
        a, b, tx, c, d, ty = matrix[:6]
        determinant = a * d - b * c
        x, y = device[0] - tx, device[1] - ty
        self.assertEqual([(d * x - b * y) / determinant, (-c * x + a * y) / determinant],
                         self.expected('transform', 'roundtrip-point'))
        values = self.operands('insert')
        self.assertEqual(transform(values['outer-transform'], transform(values['inner-transform'], values['points'][0])),
                         self.expected('insert', 'transformed-point'))
        values = self.operands('transform', 'large-origin')
        origin, points = values['origin'], values['local-points']
        world = [[origin[d] + p[d] for d in (0, 1)] for p in points]
        self.assertEqual([(world[1][d] - world[0][d]) * values['pixels-per-model'] for d in (0, 1)],
                         self.expected('transform', 'device-delta', 'large-origin'))

    def test_original_png_pixels_affine_corners_and_padded_readback(self):
        values = self.operands('image')
        raw = base64.b64decode(values['png-base64'], validate=True)
        width, height, rgba = inventory.decode_png(raw)
        self.assertEqual([width, height], values['size'])
        self.assertEqual(list(rgba), self.expected('image', 'decoded-rgba'))
        self.assertEqual('f47327f4736508c64ad289abb041f3dad0e7a33977b496600c2441bacdf5d7ef', inventory.sha256(raw))
        for variant in ('ordinary', 'mirrored'):
            values = self.operands('image', variant)
            u, v = values['uv-basis']
            x, y = values['insertion']
            result = [[x + s * u[0] + t * v[0], y + s * u[1] + t * v[1]]
                      for s, t in ((0, 0), (width, 0), (width, height), (0, height))]
            self.assertEqual(result, self.expected('image', 'corners', variant))
        values = self.operands('readback')
        rows = bytes(values['padded-rgba'])
        result = b''.join(rows[y * values['stride']:y * values['stride'] + width * 4] for y in range(height))
        self.assertEqual(list(result), self.expected('readback', 'normalized-rgba'))
        self.assertEqual(rgba, result)

    def test_asset_cache_bindings_keep_r3_deadline_methods_and_real_consumers(self):
        cases = {c['id']: c for c in self.inputs['manifest']['cases']}
        for ident, feature, variants in (
                ('text', 'F5', ('cache-retained', 'reload', 'eviction', 'owner-recreate')),
                ('image', 'F6', ('retained', 'reload', 'eviction', 'owner-recreate'))):
            case = cases[ident]
            self.assertEqual('R3', case['stage'])
            self.assertEqual({feature, 'F8', 'F12'}, set(case['features']))
            self.assertTrue(set(cases['cache']['methods']) <= set(case['methods']))
            self.assertTrue({'lcviewernoqt-documentcanvas-cpp', 'lcviewernoqt-documentcanvas-h',
                             'lcviewernoqt-drawitems-lcvdrawitem-cpp',
                             'lcviewernoqt-drawitems-lcvdrawitem-h'} <= set(case['consumers']))
            bindings = {v['id']: v for v in case['variants']}
            for variant in variants:
                binding = bindings[variant]
                suffix = ''.join(part.title() for part in variant.split('-'))
                self.assertEqual('Render' + ident.title() + 'Test.' + ident.title() + suffix, binding['test'])
                self.assertEqual('success', binding['expect'])
                self.assertEqual(0, self.expected(ident, 'settled-live-resources', variant))
        self.assertEqual('R2', cases['cache']['stage'])  # Do not move the geometry prerequisite.

    def test_image_reload_has_distinct_independent_pixels_and_retained_old_snapshot(self):
        values = self.operands('image', 'reload')
        old = inventory.decode_png(base64.b64decode(values['png-base64'], validate=True))
        new = inventory.decode_png(base64.b64decode(values['replacement-png-base64'], validate=True))
        self.assertEqual((3, 2), old[:2])
        self.assertEqual(old[:2], new[:2])
        self.assertEqual(list(old[2][:4]), self.expected('image', 'retained-opaque-pixel', 'reload'))
        self.assertEqual(list(new[2][:4]), self.expected('image', 'current-opaque-pixel', 'reload'))
        self.assertNotEqual(old[2][:4], new[2][:4])
        self.assertEqual(old[2][4:], new[2][4:])
        operations = values['operations']
        self.assertLess(operations.index('record-cached-and-uncached-image-revision-1'),
                        operations.index('overwrite-same-file-and-notify-revision-2'))
        self.assertLess(operations.index('overwrite-same-file-and-notify-revision-2'),
                        operations.index('submit-old-and-current-snapshots'))
        self.assertEqual([1, 2], values['asset-revisions'])
        self.assertIs(True, self.expected('image', 'cache-invalidated-for-new-revision', 'reload'))

    def test_asset_eviction_inputs_cross_small_limits_without_evicting_pinned_content(self):
        text = self.operands('text', 'eviction')
        self.assertEqual([72, 233, 128578], text['codepoints'])
        self.assertEqual('visible-notdef', text['fallback-policy'])
        self.assertEqual(1, text['test-page-glyph-capacity'])
        self.assertEqual(2, text['page-limit'])
        self.assertGreater(len(text['codepoints']), text['page-limit'] * text['test-page-glyph-capacity'])
        self.assertEqual(text['page-limit'], self.expected('text', 'peak-live-pages', 'eviction'))
        self.assertIs(False, self.expected('text', 'pinned-page-evicted', 'eviction'))
        image = self.operands('image', 'eviction')
        decoded = [inventory.decode_png(base64.b64decode(png, validate=True)) for png in image['png-payloads']]
        self.assertEqual(3, len(set(image['file-names'])))
        self.assertEqual([(3, 2)] * 3, [row[:2] for row in decoded])
        self.assertEqual(48, image['decoded-byte-limit'])
        self.assertEqual(image['decoded-byte-limit'], len(decoded[0][2]) + len(decoded[-1][2]))
        self.assertGreater(sum(len(row[2]) for row in decoded), image['decoded-byte-limit'])
        self.assertEqual(list(decoded[0][2][:4]), self.expected('image', 'pinned-opaque-pixel', 'eviction'))
        self.assertEqual(list(decoded[-1][2][:4]), self.expected('image', 'incoming-opaque-pixel', 'eviction'))
        self.assertIs(False, self.expected('image', 'pinned-image-evicted', 'eviction'))
        for ident in ('text', 'image'):
            self.assertEqual('injected-renderer-policy', self.operands(ident, 'eviction')['limit-kind'])
            self.assertEqual(1, self.expected(ident, 'eviction-count', 'eviction'))
            self.assertIs(True, self.expected(ident, 'cached-uncached-equal', 'eviction'))

    def test_font_reload_preserves_old_metrics_and_invalidates_future_cache_lookup(self):
        values = self.operands('text', 'reload')
        ordinary = self.operands('text')
        raw = self.expected('text', 'raw-advances')[0]
        self.assertEqual(ordinary['font-asset'], values['font-asset'])
        self.assertEqual('injected-asset-revision', values['revision-kind'])
        self.assertEqual([1, 2], values['asset-revisions'])
        for size, observed in ((values['font-size'], 'retained-model-advance'),
                               (values['next-font-size'], 'current-model-advance')):
            self.assertAlmostEqual(raw * size / values['units-per-em'], self.expected('text', observed, 'reload'))
        self.assertNotEqual(values['font-size'], values['next-font-size'])
        self.assertIs(True, self.expected('text', 'cache-invalidated-for-new-revision', 'reload'))
        self.assertIs(True, self.expected('text', 'old-and-new-glyph-readback-have-ink', 'reload'))
        operations = values['operations']
        self.assertLess(operations.index('reload-same-pinned-font-revision-2'), operations.index('submit-old-snapshot'))
        self.assertLess(operations.index('submit-old-snapshot'), operations.index('record-cached-and-uncached-h-revision-2'))
        self.assertAlmostEqual(raw * ordinary['font-size'] / ordinary['units-per-em'],
                               self.expected('text', 'recorded-model-advance', 'cache-retained'))

    def test_asset_owner_recreation_cannot_substitute_global_generation_or_hide_survivor(self):
        for ident in ('text', 'image'):
            values = self.operands(ident, 'owner-recreate')
            self.assertEqual(['device-a', 'device-b'], values['owners'])
            self.assertEqual([1, 1], values['owner-generations'])
            self.assertEqual(42, values['entity-id'])
            operations = values['operations']
            self.assertLess(operations.index('drain-both'), operations.index('destroy-owner-a'))
            self.assertLess(operations.index('destroy-owner-a'), operations.index('readback-surviving-owner-b'))
            self.assertLess(operations.index('readback-surviving-owner-b'), operations.index('recreate-owner-a-generation-2'))
            self.assertEqual(2, self.expected(ident, 'recreated-owner-generation', 'owner-recreate'))
            self.assertEqual(1, self.expected(ident, 'unaffected-owner-generation', 'owner-recreate'))
            self.assertIs(False, self.expected(ident, 'cross-device-resource-sharing', 'owner-recreate'))
            self.assertIs(True, self.expected(ident, 'survivor-readback-unchanged', 'owner-recreate'))
            self.assertIs(True, self.expected(ident, 'recreated-cached-uncached-equal', 'owner-recreate'))
        # This checks proposed operands/order, not native device teardown or observed GPU resources.

    def test_repeated_image_acquisition_and_public_handle_release_keep_queued_snapshots(self):
        values = self.operands('image', 'retained')
        self.assertEqual(1000, values['acquire-count'])
        self.assertEqual(1, self.expected('image', 'decode-count', 'retained'))
        operations = values['operations']
        self.assertLess(operations.index('record-cached-and-uncached-image'), operations.index('destroy-public-handle'))
        self.assertLess(operations.index('destroy-public-handle'), operations.index('submit-retained-snapshots'))
        self.assertIs(True, self.expected('image', 'retained-image-visible', 'retained'))
        self.assertIs(True, self.expected('image', 'cached-uncached-equal', 'retained'))
        decoded = inventory.decode_png(base64.b64decode(values['png-base64'], validate=True))[2]
        self.assertEqual(list(decoded[:4]), self.expected('image', 'retained-opaque-pixel', 'retained'))

    def test_blend_reference_not_legacy_pixels(self):
        for variant, background, colors in (
                ('ordinary', [0, 0, 0, 1], [[1, 0, 0, .5], [0, 0, 1, .5]]),
                ('transparent', [0, 0, 0, 0], [[1, 0, 0, .5], [0, 0, 1, .5]]),
                ('ordered-clear', [0, 1, 0, 1], [[0, 0, 1, .5]])):
            alpha = background[3]
            rgb = [c * alpha for c in background[:3]]
            for source in colors:
                rgb = [source[i] * source[3] + rgb[i] * (1 - source[3]) for i in range(3)]
                alpha = source[3] + alpha * (1 - source[3])
            straight = [c / alpha for c in rgb] + [alpha]
            for got, expected in zip(straight, self.expected('layers', 'overlap-rgba', variant)):
                self.assertLessEqual(abs(round(255 * got) - expected), 1)

    def test_pinned_font_raw_cmap_metrics_and_embedded_notice(self):
        fixture = self.inputs['fixtures']['abeezee']
        font = (inventory.REPO_ROOT / fixture['path']).read_bytes()
        def u16(data, offset):
            return struct.unpack_from('>H', data, offset)[0]
        tables = {}
        for n in range(u16(font, 4)):
            tag, _, offset, size = struct.unpack_from('>4sIII', font, 12 + n * 16)
            self.assertLessEqual(offset + size, len(font))
            tables[tag] = font[offset:offset + size]
        # Deliberately a known-fixture OpenType check, not a production font parser.
        self.assertEqual(self.operands('text')['units-per-em'], u16(tables[b'head'], 18))
        cmap = tables[b'cmap']
        subtables = [struct.unpack_from('>HHI', cmap, 4 + i * 8) for i in range(u16(cmap, 2))]
        offset = next(o for platform, encoding, o in subtables if platform == 3 and encoding == 1)
        self.assertEqual(4, u16(cmap, offset))
        table = cmap[offset:offset + u16(cmap, offset + 2)]
        segments = u16(table, 6) // 2
        def glyph(codepoint):
            for i in range(segments):
                end, start = u16(table, 14 + i * 2), u16(table, 16 + 2 * segments + i * 2)
                if start <= codepoint <= end:
                    delta = u16(table, 16 + 4 * segments + i * 2)
                    location = 16 + 6 * segments + i * 2
                    displacement = u16(table, location)
                    value = u16(table, location + displacement + 2 * (codepoint - start)) if displacement else codepoint
                    return (value + delta) & 0xffff if value or not displacement else 0
            return 0
        for variant, codepoint in (('ordinary', 72), ('latin', 233), ('supplementary', 128578)):
            index = glyph(codepoint)
            self.assertEqual([index], self.expected('text', 'glyph-ids', variant))
            self.assertLess(index, u16(tables[b'hhea'], 34))
            advance, bearing = struct.unpack_from('>Hh', tables[b'hmtx'], index * 4)
            expected = self.expected('text', 'raw-advances' if variant == 'ordinary' else 'raw-advance', variant)
            self.assertEqual([advance] if variant == 'ordinary' else advance, expected)
            if variant == 'ordinary':
                self.assertEqual([bearing], self.expected('text', 'raw-left-bearings'))
                self.assertAlmostEqual(advance / 1000 * self.operands('text')['font-size'],
                                       self.expected('text', 'model-advance'))
        name = tables[b'name']
        strings = {}
        for n in range(u16(name, 2)):
            platform, encoding, language, ident, size, offset = struct.unpack_from('>HHHHHH', name, 6 + n * 12)
            if platform == 3:
                start = u16(name, 4) + offset
                strings[ident] = name[start:start + size].decode('utf-16-be')
        self.assertIn('1.002', strings[5])
        self.assertIn('Anja Meiners', strings[0])
        self.assertIn('SIL Open Font License, Version 1.1', strings[13])
        notice = (inventory.REPO_ROOT / fixture['notice']).read_text()
        self.assertIn(strings[0], notice)

    def test_all_dimension_family_measurements_and_tick_geometry(self):
        for ident in ('dimension-aligned', 'dimension-linear', 'dimension-angular',
                      'dimension-radial', 'dimension-diametric'):
            values = self.operands(ident)
            points = values['points']
            if ident == 'dimension-angular':
                a, center, b = points
                first = math.atan2(a[1] - center[1], a[0] - center[0])
                second = math.atan2(b[1] - center[1], b[0] - center[0])
                measurement = abs(math.remainder(second - first, math.tau))
            elif ident == 'dimension-linear':
                a, b = points
                axis = values['measurement-axis']
                measurement = abs(sum((b[d] - a[d]) * axis[d] for d in (0, 1))) / math.hypot(*axis)
            else:
                a, b = points
                measurement = math.hypot(b[0] - a[0], b[1] - a[1])
            self.assertAlmostEqual(measurement, self.expected(ident, 'measurement'))
        values = self.operands('endcaps')
        header = (inventory.REPO_ROOT / 'lcviewernoqt/drawitems/endcaps.h').read_text()
        for name, number in zip(values['cap-names'], values['cap-types']):
            self.assertRegex(header, r'\b' + name + r'\s*=\s*' + str(number) + r'\b')
        delta = [values['size'] / 2 * f(values['tick-angle']) for f in (math.cos, math.sin)]
        result = [[values['end'][d] + sign * delta[d] for d in (0, 1)] for sign in (-1, 1)]
        for actual, expected in zip(result, self.expected('endcaps', 'tick-endpoints')):
            for a, b in zip(actual, expected):
                self.assertLessEqual(abs(a - b), self.inputs['oracles']['model-error']['limit'])

    def test_gradient_landmarks_grid_paper_and_uniform_placements(self):
        values = self.operands('gradient')
        result = []
        a, b = values['axis']
        offsets, colors = values['offsets'], values['colors']
        for point in values['sample-points']:
            t = (point[0] - a[0]) / (b[0] - a[0])
            index = next(i for i in range(len(offsets) - 1) if offsets[i] <= t <= offsets[i + 1])
            local = (t - offsets[index]) / (offsets[index + 1] - offsets[index])
            result.extend(round(colors[index * 4 + c] * (1 - local) + colors[(index + 1) * 4 + c] * local)
                          for c in range(4))
        self.assertEqual(result, self.expected('gradient', 'landmark-rgba'))
        values = self.operands('grid')
        x0, y0, x1, y1 = values['bounds']
        for low, high, field in ((x0, x1, 'vertical-grid-positions'), (y0, y1, 'horizontal-grid-positions')):
            self.assertEqual(list(range(low, high + 1, values['spacing'])), self.expected('grid', field))
        values = self.operands('paper')
        x, y = values['origin']
        width, height = values['size']
        self.assertEqual([[x, y], [x + width, y], [x + width, y + height], [x, y + height]],
                         self.expected('paper', 'corners'))
        values = self.operands('uniform-ranges')
        for key, probe in zip(('transform-a', 'transform-b', 'transform-c'), values['probes']):
            m = values[key]
            self.assertEqual([m[0] + m[1] + m[2], m[3] + m[4] + m[5]], probe)

    def test_nonempty_text_requires_coverage_not_metrics_only(self):
        for ident in ('text', 'app-canvas-smoke'):
            values = self.operands(ident)
            region = values['glyph-probe-region']
            self.assertLess(region[0], region[2])
            self.assertLess(region[1], region[3])
            self.assertTrue(self.expected(ident, 'glyph-readback-has-ink'))
        self.assertTrue(self.operands('text')['metrics-query-before-frame'])
        self.assertEqual(['ABeeZee'], self.expected('text', 'font-list-contains'))
        # This guards the requirement, not an observed native glyph draw.

    def test_utf8_operands_and_python_program_syntax_without_execution(self):
        for variant in ('ordinary', 'latin', 'supplementary'):
            raw = bytes(self.operands('text', variant)['utf8-bytes'])
            decoded = [ord(c) for c in raw.decode('utf-8')]
            expected = [72] if variant == 'ordinary' else self.expected('text', 'decoded-codepoints', variant)
            self.assertEqual(expected, decoded)
        with self.assertRaises(UnicodeDecodeError):
            bytes(self.operands('text', 'invalid-utf8')['utf8-bytes']).decode('utf-8')
        script = self.operands('cli-python')['script']
        compile(script, '<renderer-contract>', 'exec')  # Syntax only, never exec().
        self.assertEqual(['lc', 'document'], self.operands('cli-python')['globals'])

    def test_workload_counts_generator_and_cycle_sampling(self):
        for ident, variant in (('workload-mixed', 'ordinary'), ('workload-mixed', 'dense')):
            values = self.operands(ident, variant)
            self.assertEqual(values['entity-count'], self.expected(ident, 'generated-entity-count', variant))
            self.assertEqual(2 ** 32, values['modulus'])
            state, positions = values['seed'], []
            for _ in range(3):
                state = (values['multiplier'] * state + values['increment']) % values['modulus']
                x = state % values['coordinate-modulus']
                state = (values['multiplier'] * state + values['increment']) % values['modulus']
                positions.append([x, state % values['coordinate-modulus']])
            self.assertEqual([[276, 355], [870, 909], [904, 327]], positions)
            self.assertEqual(positions, self.expected(ident, 'first-positions', variant))
        self.assertEqual(sum(self.operands('workload-assets')['kind-counts']),
                         self.expected('workload-assets', 'generated-entity-count'))
        lifecycle = next(w for w in self.inputs['manifest']['workloads'] if w['id'] == 'lifecycle')
        self.assertEqual(200, lifecycle['samples'] * self.operands('workload-lifecycle')['cycles-per-sample'])
        self.assertEqual(30, self.operands('workload-idle')['observe-seconds'])

    def test_pinned_bytes_have_narrow_checkout_attributes(self):
        manifest = inventory.CASE_PATH.relative_to(inventory.REPO_ROOT).as_posix()
        names = [f['path'] for f in self.inputs['manifest']['fixtures']] + [HEADER_PATH, manifest, 'unittest/unrelated.dxf']
        result = subprocess.run(['git', 'check-attr', '-z', 'text', 'eol', 'diff', '--', *names],
                                cwd=inventory.REPO_ROOT, capture_output=True, check=True)
        fields = result.stdout.decode().split('\0')[:-1]
        attrs = {(fields[n], fields[n + 1]): fields[n + 2] for n in range(0, len(fields), 3)}
        for path in (self.inputs['manifest']['fixtures'][0]['path'], manifest, HEADER_PATH):
            self.assertEqual('lf', attrs[path, 'eol'])
        self.assertEqual('unset', attrs['unittest/unrelated.dxf', 'diff'])
        self.assertEqual('unset', attrs['unittest/unrelated.dxf', 'text'])
        self.assertEqual('unset', attrs[self.inputs['fixtures']['abeezee']['path'], 'text'])


class SelectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.real = inventory.validate_case_manifest()

    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.path = Path(temp.name) / 'cases.json'
        self.manifest = copy.deepcopy(self.real['manifest'])

    def validate(self):
        self.path.write_bytes(json.dumps(self.manifest).encode())
        return inventory.validate_case_manifest(self.path)

    def config(self, ident):
        return next(c for c in self.manifest['configurations'] if c['id'] == ident)

    def case(self, ident):
        return next(c for c in self.manifest['cases'] if c['id'] == ident)

    def select(self, stage, key):
        return inventory.select_case_requirements(self.real, stage, key)

    def cli(self, *args):
        return subprocess.run([sys.executable, '-B', str(inventory.REPO_ROOT / 'scripts/validate_renderer_inventory.py'),
                               *map(str, args)], text=True, capture_output=True, timeout=20)

    def test_real_inventory_relations_and_not_execution(self):
        self.assertEqual(189, len(self.real['configurations']))
        self.assertEqual(223, len(self.real['requirements']))
        summary = inventory.case_inventory_summary(self.real)
        self.assertFalse(summary['execution_evidence'])
        self.assertEqual('renderer-case-inventory', summary['evidence_kind'])
        self.assertEqual(sorted(self.real['configurations']), summary['configurations'])
        self.assertEqual(set(self.real['configurations']),
                         {key for row in self.real['requirements'] for key in row['configurations']})

    def test_profiles_explicitly_define_python_widgets_cli_and_test_intent(self):
        for name in ('rhi-only', 'dual', 'rhi-offscreen', 'rhi-cli', 'dual-legacy', 'cpu', 'cairo'):
            self.assertIs(False, inventory.profile_options(name)['WITH_PYTHONSCRIPT'])
        for name in ('scripted', 'rhi-cli-python'):
            self.assertIs(True, inventory.profile_options(name)['WITH_PYTHONSCRIPT'])
        for name in ('rhi-offscreen', 'rhi-cli', 'rhi-cli-python', 'cpu', 'cairo'):
            self.assertIs(False, inventory.profile_options(name)['WITH_QT_UI'])
        self.assertIs(False, inventory.profile_options('rhi-offscreen')['WITH_LUACMDINTERFACE'])
        self.assertIs(True, inventory.profile_options('rhi-cli')['WITH_LUACMDINTERFACE'])
        self.assertIs(True, inventory.profile_options('dual-legacy')['WITH_LEGACY_OPENGL_RENDERER'])
        self.assertIs(False, inventory.profile_options('cpu')['WITH_RENDERER_TESTS'])
        for profile in (None, 'automatic', 'unknown'):
            with self.assertRaises(ValueError):
                inventory.profile_options(profile)

    def test_invalid_configuration_values_fail_safely(self):
        original = copy.deepcopy(self.manifest['configurations'])
        for field, value in (('platforms', ['UNKNOWN']), ('modes', ['headless']), ('qpas', ['offscreen']),
                             ('devices', ['unknown']), ('profiles', ['automatic']), ('backend', False),
                             ('artifact', []), ('artifact', 'deb'), ('modes', []), ('id', '../unsafe')):
            self.manifest['configurations'] = copy.deepcopy(original)
            self.config('source-linux')[field] = value
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                inventory.case_configurations(self.manifest['configurations'])

    def test_duplicate_ids_logical_tuples_and_dead_mode_fields_rejected(self):
        original = copy.deepcopy(self.manifest['configurations'])
        duplicate = copy.deepcopy(self.config('cpu-source'))
        self.manifest['configurations'].append(duplicate)
        with self.assertRaisesRegex(ValueError, 'duplicate configuration ID'):
            inventory.case_configurations(self.manifest['configurations'])
        duplicate['id'] = 'cpu-alias'
        with self.assertRaisesRegex(ValueError, 'duplicate logical configuration'):
            inventory.case_configurations(self.manifest['configurations'])
        self.manifest['configurations'] = original
        self.config('gui-less-linux')['modes'].append('widget')
        with self.assertRaisesRegex(ValueError, 'unreachable configuration field'):
            inventory.case_configurations(self.manifest['configurations'])

    def test_active_api_legacy_device_and_cpu_boundaries(self):
        original = copy.deepcopy(self.manifest['configurations'])
        for ident, field, value in (('source-windows', 'backend', 'direct3d11'),
                                    ('source-linux', 'devices', ['any']),
                                    ('source-macos', 'backend', 'vulkan'),
                                    ('legacy-linux', 'backend', 'vulkan'),
                                    ('legacy-windows', 'devices', ['hardware']),
                                    ('cpu-source', 'qpas', ['xcb'])):
            self.manifest['configurations'] = copy.deepcopy(original)
            self.config(ident)[field] = value
            with self.subTest(ident=ident, field=field), self.assertRaises(ValueError):
                inventory.case_configurations(self.manifest['configurations'])

    def test_windows_cli_and_unsupported_package_architectures_rejected(self):
        self.config('gui-less-windows')['profiles'].append('rhi-cli')
        with self.assertRaisesRegex(ValueError, 'Windows CLI'):
            inventory.case_configurations(self.manifest['configurations'])
        self.manifest = copy.deepcopy(self.real['manifest'])
        self.config('appimage')['platforms'].append('LINUX-ARM64')
        with self.assertRaisesRegex(ValueError, 'package architecture'):
            inventory.case_configurations(self.manifest['configurations'])

    def test_missing_coverage_layers_are_not_missing_runner_exemptions(self):
        original = copy.deepcopy(self.manifest['configurations'])
        for ident, field, value in (('source-linux', 'devices', ['hardware']),
                                    ('source-linux', 'devices', ['software']),
                                    ('source-macos', 'profiles', ['dual', 'scripted']),
                                    ('install-linux', 'profiles', ['scripted']),
                                    ('gui-less-linux', 'profiles', ['rhi-offscreen', 'rhi-cli']),
                                    ('legacy-linux', 'platforms', ['LINUX-X64'])):
            self.manifest['configurations'] = copy.deepcopy(original)
            self.config(ident)[field] = value
            with self.subTest(ident=ident, field=field), self.assertRaises(ValueError):
                inventory.case_configurations(self.manifest['configurations'])
        self.manifest['configurations'] = [c for c in original if c['id'] != 'cairo']
        with self.assertRaisesRegex(ValueError, 'Cairo layer'):
            inventory.case_configurations(self.manifest['configurations'])

    def test_packages_do_not_require_every_source_device_cross_product(self):
        artifacts = {c['id']: c['artifact'] for c in self.manifest['configurations']}
        for config in self.manifest['configurations']:
            if config['artifact'] != 'source':
                config['devices'] = ['hardware']
        # Explicitly assign package-only cases to that candidate route. This is
        # metadata, never approval of its D4 provider or hardware availability.
        for case in self.manifest['cases']:
            if all(artifacts[group] != 'source' for group in case['configurations']):
                case['devices'] = ['hardware']
        result = self.validate()
        self.assertLess(len(result['configurations']), len(self.real['configurations']))
        self.assertFalse(inventory.case_inventory_summary(result)['execution_evidence'])

    def test_installed_profiles_do_not_add_dual_or_widgets_off_test_hosts(self):
        for profile in ('dual', 'rhi-offscreen'):
            self.manifest = copy.deepcopy(self.real['manifest'])
            self.config('install-linux')['profiles'].append(profile)
            with self.subTest(profile=profile), self.assertRaisesRegex(ValueError, 'installed/package profiles'):
                inventory.case_configurations(self.manifest['configurations'])

    def test_python_prerequisites_cannot_be_routed_to_python_off(self):
        self.case('python-callbacks')['profiles'].append('rhi-only')
        with self.assertRaisesRegex(ValueError, 'prerequisite conflicts'):
            self.validate()
        self.manifest = copy.deepcopy(self.real['manifest'])
        self.case('cli-python')['requires'].remove('python')
        with self.assertRaisesRegex(ValueError, 'explicit Python prerequisite'):
            self.validate()

    def test_rhi_cases_cannot_use_a_dual_builds_active_legacy_backend(self):
        case = self.case('line')
        case.update(configurations=['legacy-linux', 'legacy-windows', 'legacy-macos'], profiles=['dual-legacy'],
                    modes=['widget'], devices=['any'])
        with self.assertRaisesRegex(ValueError, 'active backend'):
            self.validate()

    def test_unreachable_case_requests_and_late_feature_coverage_fail(self):
        self.case('line')['modes'].append('cpu')
        with self.assertRaisesRegex(ValueError, 'unreachable case selection'):
            self.validate()
        self.manifest = copy.deepcopy(self.real['manifest'])
        for case in self.manifest['cases']:
            if 'F5' in case['features']:
                case['stage'] = 'R7'
        with self.assertRaisesRegex(ValueError, 'missing timely'):
            self.validate()

    def test_software_consumer_holes_cannot_hide_in_hardware_totals(self):
        ident = next(key for key, c in self.real['consumers'].items() if c['source'] == 'lcviewernoqt/drawitems/lcvline.cpp')
        for case in self.manifest['cases']:
            if ident in case['consumers'] and 'software' in case['devices']:
                case['devices'] = ['hardware']
        with self.assertRaisesRegex(ValueError, 'missing timely consumer'):
            self.validate()

    def test_cpu_only_selection_contains_real_recording_requirements(self):
        selection = self.select('R2', 'cpu-source/LINUX-ARM64/cpu/cpu/none/none')
        cases = {r['id'].rsplit('.', 1)[0] for r in selection['requirements']}
        self.assertEqual({'cpu-build-options', 'cpu-path-state', 'cpu-transform', 'cpu-spline', 'cpu-hatch-fill'}, cases)
        self.assertFalse(selection['configuration']['expected_options']['WITH_RHI_RENDERER'])
        self.assertFalse(selection['configuration']['expected_options']['WITH_QT_UI'])

    def test_r1_pixel_and_stage_prefix_selection_excludes_future_cases(self):
        key = 'source-linux/LINUX-X64/rhi-only/offscreen/xcb/software'
        first = self.select('R1', key)
        self.assertEqual({'rhi-build-options'}, {r['id'].split('.')[0] for r in first['requirements']})
        earlier = set()
        for stage in ('R1', 'R2', 'R3', 'R4', 'R5', 'R6', 'R7'):
            result = self.select(stage, key)
            ids = {r['id'] for r in result['requirements']}
            self.assertTrue(earlier <= ids)
            self.assertTrue(all(int(r['stage'][1]) <= int(stage[1]) for r in result['requirements']))
            self.assertEqual(sorted(ids), [r['id'] for r in result['requirements']])
            earlier = ids
        self.assertNotIn('default-policy.ordinary', {r['id'] for r in self.select('R5', key)['requirements']})

    def test_r1_bootstrap_geometry_and_pixels_have_an_independent_cpu_check(self):
        body = self.real['datasets']['contracts']['rhi-build-options']['ordinary']
        values = {key: record['value'] for key, record in body['inputs'].items()}
        width, height = values['size']
        vertices, indices = values['vertices'], values['indices']
        area = 0
        for offset in range(0, len(indices), 3):
            a, b, c = [vertices[i] for i in indices[offset:offset + 3]]
            area += abs((b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])) / 2
        self.assertEqual(width * height, area)
        self.assertNotEqual(values['rgba'], values['background-rgba'])
        expected = next(a['expected']['value'] for a in body['assertions'] if a['observed'] == 'normalized-rgba')
        self.assertEqual(values['rgba'] * (width * height), expected)
        self.assertEqual(1, values['sample-count'])
        self.assertEqual('RGBA8', values['format'])
        self.assertEqual('top-left', values['output-origin'])
        limit = self.real['datasets']['contracts']['rhi-build-options']['limit']['inputs']
        for key in ('texture-limit', 'requested-width'):
            self.assertEqual('physical-pixel', limit[key]['unit'])
        self.assertLess(limit['texture-limit']['value'], limit['requested-width']['value'])
        self.assertEqual('injected-renderer-policy', limit['limit-kind']['value'])
        # This verifies data, not an observed RHI draw or native API.

    def test_r3_asset_cache_bindings_cover_source_tuples_but_not_r2_or_package_gtests(self):
        expected = {'text.' + name for name in ('cache-retained', 'reload', 'eviction', 'owner-recreate')}
        expected |= {'image.' + name for name in ('retained', 'reload', 'eviction', 'owner-recreate')}
        platforms = set()
        for key, config in self.real['configurations'].items():
            if config['artifact'] != 'source' or config['backend'] not in ('vulkan', 'metal'):
                continue
            selected = self.select('R3', key)
            platforms.add(config['platform'])
            self.assertTrue(expected <= {row['id'] for row in selected['requirements']}, key)
            self.assertTrue(all(row['stage'] == 'R3' for row in selected['requirements'] if row['id'] in expected))
            self.assertFalse(selected['execution_evidence'])
            if config['mode'] == 'offscreen':
                self.assertFalse(expected & {row['id'] for row in self.select('R2', key)['requirements']}, key)
        self.assertEqual(set(inventory.CASE_PLATFORMS), platforms)
        package = self.select('R5', 'snap/LINUX-X64/scripted/widget/xcb/software')
        self.assertFalse(expected & {row['id'] for row in package['requirements']})
        self.assertTrue(all(row['binding'] == 'proposed-runner-check' for row in package['requirements']))

    def test_mode_artifact_and_retirement_stage_boundaries(self):
        for stage, key in (
                ('R2', 'source-linux/LINUX-X64/rhi-only/widget/xcb/software'),
                ('R4', 'snap/LINUX-X64/scripted/widget/xcb/software'),
                ('R7', 'source-linux/LINUX-X64/dual/offscreen/xcb/software'),
                ('R7', 'legacy-linux/LINUX-X64/dual-legacy/widget/xcb/any'),
                ('R1', 'cairo/LINUX-X64/cairo/cpu/none/none')):
            with self.subTest(stage=stage, key=key), self.assertRaises(ValueError):
                self.select(stage, key)

    def test_python_cli_and_widget_selection_are_separate(self):
        cases = lambda selection: {r['id'].split('.')[0] for r in selection['requirements']}
        off = self.select('R4', 'gui-less-linux/LINUX-X64/rhi-cli/offscreen/wayland/software')
        on = self.select('R4', 'gui-less-linux/LINUX-X64/rhi-cli-python/offscreen/wayland/software')
        no_cli = self.select('R4', 'gui-less-linux/LINUX-X64/rhi-offscreen/offscreen/wayland/software')
        self.assertIn('cli-lua', cases(off))
        self.assertNotIn('cli-python', cases(off))
        self.assertIn('cli-python-disabled', cases(off))
        self.assertNotIn('cli-python-disabled', cases(on))
        self.assertTrue({'cli-lua', 'cli-python'} <= cases(on))
        self.assertFalse({'cli-lua', 'cli-python', 'widget-routing'} & cases(no_cli))
        widget = self.select('R3', 'source-windows/WIN-X64/scripted/widget/windows/software')
        self.assertTrue({'python-callbacks', 'lua-callbacks', 'widget-routing'} <= cases(widget))

    def test_package_smoke_is_runner_only_and_performance_hardware_only(self):
        for device in ('software', 'hardware'):
            selection = self.select('R5', 'snap/LINUX-X64/scripted/widget/xcb/' + device)
            self.assertTrue(all(r['binding'] == 'proposed-runner-check' for r in selection['requirements']))
            workloads = [r for r in selection['requirements'] if r['id'].startswith('workload-')]
            self.assertEqual(device == 'hardware', bool(workloads))
            self.assertFalse(selection['execution_evidence'])

    def test_legacy_process_and_cairo_keep_their_real_identity(self):
        legacy = self.select('R6', 'legacy-linux/LINUX-X64/dual-legacy/widget/wayland/any')
        self.assertEqual('legacy-opengl', legacy['configuration']['backend'])
        self.assertEqual({'legacy-compatibility'}, {r['id'].split('.')[0] for r in legacy['requirements']})
        cairo = self.select('R4', 'cairo/LINUX-X64/cairo/cpu/none/none')
        self.assertEqual('cairo', cairo['configuration']['backend'])
        self.assertEqual({'cairo-compatibility'}, {r['id'].split('.')[0] for r in cairo['requirements']})

    def test_every_declared_stage_tuple_has_exact_selection_or_explicit_error(self):
        successful, refused = 0, 0
        for key, config in self.real['configurations'].items():
            for number in range(1, 8):
                stage = 'R' + str(number)
                expected = [r for r in self.real['requirements'] if key in r['configurations'] and int(r['stage'][1]) <= number]
                invalid = (not expected or config['mode'] == 'widget' and number < 3 or
                           config['artifact'] != 'source' and number < 5 or
                           number == 7 and config['expected_options']['WITH_LEGACY_OPENGL_RENDERER'])
                if invalid:
                    with self.subTest(stage=stage, key=key), self.assertRaises(ValueError):
                        self.select(stage, key)
                    refused += 1
                else:
                    actual = self.select(stage, key)
                    self.assertEqual([r['id'] for r in expected], [r['id'] for r in actual['requirements']])
                    self.assertTrue(all(r['test'] and r['fixture'] in actual['fixtures'] for r in actual['requirements']))
                    successful += 1
        self.assertEqual(len(self.real['configurations']) * 7, successful + refused)
        self.assertGreater(successful, 0)
        self.assertGreater(refused, 0)

    def test_unknown_stage_tuple_and_old_requirements_schema_fail(self):
        for stage, key in (('R0', next(iter(self.real['configurations']))), ('R2', 'unknown')):
            with self.assertRaises(ValueError):
                self.select(stage, key)
        self.manifest['schema_version'] = 1
        with self.assertRaisesRegex(ValueError, 'case-manifest schema'):
            self.validate()

    def test_manifest_snapshot_digest_binds_the_bytes_actually_parsed(self):
        data = inventory.CASE_PATH.read_bytes()
        self.assertEqual(inventory.sha256(data), self.real['manifest_sha256'])
        original = inventory._bounded_bytes
        reads = []
        def read(path, limit):
            if path == inventory.CASE_PATH:
                reads.append(path)
                return data if len(reads) == 1 else b'{}'
            return original(path, limit)
        with patch.object(inventory, '_bounded_bytes', side_effect=read):
            result = inventory.validate_case_manifest()
        self.assertEqual(1, len(reads))
        self.assertEqual(inventory.sha256(data), result['manifest_sha256'])

    def test_cli_inventory_and_selection_are_deterministic_json_not_reports(self):
        args = ('--cases', inventory.CASE_PATH)
        result = self.cli(*args)
        self.assertEqual(0, result.returncode, result.stderr)
        summary = json.loads(result.stdout)
        self.assertEqual('renderer-case-inventory', summary['evidence_kind'])
        self.assertFalse(summary['execution_evidence'])
        self.assertEqual(223, summary['variants'])
        args += ('--stage', 'R2', '--configuration', 'source-linux/LINUX-X64/rhi-only/offscreen/xcb/software')
        result, repeat = self.cli(*args), self.cli(*args)
        self.assertEqual(0, result.returncode, result.stderr)
        self.assertEqual(result.stdout, repeat.stdout)
        selection = json.loads(result.stdout)
        self.assertEqual('renderer-case-selection', selection['evidence_kind'])
        self.assertFalse(selection['execution_evidence'])
        self.path.write_bytes(result.stdout.encode())
        with self.assertRaisesRegex(ValueError, 'unsupported schema'):
            inventory.validate_report(self.path, 'vulkan', 'software', 'offscreen')

    def test_cli_rejects_mixed_missing_empty_and_unknown_requests_without_json(self):
        for suffix in (('--stage', 'R2'), ('--configuration', 'unknown'), ('--backend', 'vulkan'),
                       ('--background', 'opaque'), ('report.json',),
                       ('--stage', 'R2', '--configuration', 'unknown'),
                       ('--stage', 'R1', '--configuration', 'cairo/LINUX-X64/cairo/cpu/none/none')):
            result = self.cli('--cases', inventory.CASE_PATH, *suffix)
            with self.subTest(suffix=suffix):
                self.assertNotEqual(0, result.returncode)
                self.assertEqual('', result.stdout)
        self.assertNotEqual(0, self.cli('--stage', 'R2', '--configuration', 'unknown').returncode)

    def test_metadata_validation_never_launches_build_or_native_programs(self):
        with patch('subprocess.run', side_effect=AssertionError('external program launched')), \
                patch.object(inventory, 'validate_report', side_effect=AssertionError('render validation substituted')):
            result = inventory.validate_case_manifest()
            self.assertFalse(inventory.case_inventory_summary(result)['execution_evidence'])
            self.assertFalse(inventory.select_case_requirements(result, 'R2',
                'source-linux/LINUX-X64/rhi-only/offscreen/xcb/software')['execution_evidence'])

    def test_disabled_python_diagnostics_need_no_enabled_interpreter(self):
        result = self.select('R4', 'gui-less-linux/LINUX-X64/rhi-cli/offscreen/xcb/software')
        failures = [r for r in result['requirements'] if r['id'].startswith('cli-python-disabled.')]
        self.assertEqual(3, len(failures))
        self.assertFalse(result['configuration']['expected_options']['WITH_PYTHONSCRIPT'])
        self.assertTrue(all(r['expect'] == 'diagnostic' and r['reason'] == 'python-unavailable' for r in failures))
        script = self.real['datasets']['contracts']['cli-python-disabled']['ordinary']['inputs']['script']['value']
        compile(script, '<disabled-python-request>', 'exec')  # Syntax only.

    def test_negative_only_cases_cannot_replace_successful_feature_coverage(self):
        for case in self.manifest['cases']:
            if case['id'] != 'cli-python-disabled':
                case['features'] = [f for f in case['features'] if f != 'F11'] or ['F1']
        with self.assertRaisesRegex(ValueError, 'missing timely feature'):
            self.validate()
        key = next(k for k, c in self.real['configurations'].items() if c['backend'] == 'legacy-opengl')
        case = dict(id='diagnostic-only', stage='R4', variants=[dict(id='ordinary', expect='diagnostic')])
        with self.assertRaisesRegex(ValueError, 'successful legacy'):
            inventory._case_coverage({'diagnostic-only': case}, {'diagnostic-only': {key}},
                                     {key: self.real['configurations'][key]}, {}, set())


if __name__ == '__main__':
    unittest.main()
if __name__ == '__main__':
    unittest.main()
