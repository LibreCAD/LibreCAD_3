#!/usr/bin/env python3
"""Native-provider CMake input negatives, not renderer execution evidence."""

import argparse
import json
import shutil
import subprocess
import tempfile
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('cmake', 'generator', 'library', 'include'):
        parser.add_argument('--' + name, required=True)
    args = parser.parse_args()
    project = Path(__file__).resolve().parent
    original = project.parents[2]
    manifest_path = Path('unittest/rendering/renderer_cases.json')
    corpus_path = Path('unittest/rendering/fixtures/renderer_contracts.json')
    source_manifest = json.loads((original / manifest_path).read_text())
    source_corpus = json.loads((original / corpus_path).read_text())
    font = next(item for item in source_manifest['fixtures'] if item['id'] == 'abeezee')
    cases = [('control', None), ('font', 'Stale font/notice'), ('notice', 'Stale font/notice'),
             ('duplicate', 'Missing/ambiguous integer font oracle'),
             ('missing', 'Missing/ambiguous integer font oracle'), ('path', 'Unsafe fixture path'),
             ('utf8', 'Ordinary probe operand must be one ASCII'),
             ('empty-glyph', 'expects one glyph'), ('type', 'Wrong font oracle type/unit'),
             ('unit', 'Wrong font oracle type/unit'), ('scale', 'Invalid design-unit scale')]
    with tempfile.TemporaryDirectory(prefix='lc-font-config-') as temp:
        for index, (case, diagnostic) in enumerate(cases):
            root = Path(temp) / str(index)
            repo = root / 'repo'
            for name in ('CMakeLists.txt', 'fontprobe.cpp', 'font_data.h.in', 'check_config.py'):
                target = repo / 'lcrenderer/rhi/fontprobe' / name
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(project / name, target)
            for name in (manifest_path, corpus_path, font['path'], font['notice']):
                target = repo / name
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(original / name, target)
            manifest = json.loads(json.dumps(source_manifest))
            corpus = json.loads(json.dumps(source_corpus))
            ordinary = corpus['datasets']['text']['ordinary']
            if case == 'font':
                (repo / font['path']).write_bytes(b'changed font')
            elif case == 'notice':
                (repo / font['notice']).write_text('changed notice')
            elif case == 'duplicate':
                ordinary['assertions'].append(next(a for a in ordinary['assertions'] if a['observed'] == 'raw-advances'))
            elif case == 'missing':
                ordinary['assertions'] = [a for a in ordinary['assertions'] if a['observed'] != 'raw-advances']
            elif case == 'path':
                next(f for f in manifest['fixtures'] if f['id'] == 'abeezee')['path'] = '../outside'
            elif case == 'utf8':
                ordinary['inputs']['utf8-bytes']['value'] = [195, 169]
            elif case == 'empty-glyph':
                next(a for a in ordinary['assertions'] if a['observed'] == 'glyph-ids')['expected']['value'] = []
            elif case == 'type':
                next(a for a in ordinary['assertions'] if a['observed'] == 'raw-advances')['expected']['value'] = ['715']
            elif case == 'unit':
                next(a for a in ordinary['assertions'] if a['observed'] == 'raw-advances')['expected']['unit'] = 'physical-pixel'
            elif case == 'scale':
                ordinary['inputs']['units-per-em']['value'] = 0
            (repo / manifest_path).write_text(json.dumps(manifest))
            (repo / corpus_path).write_text(json.dumps(corpus))
            command = [args.cmake, '-S', str(repo / 'lcrenderer/rhi/fontprobe'), '-B', str(root / 'build'),
                       '-G', args.generator, '-DBUILD_TESTING=OFF', '-DFREETYPE_LIBRARY_RELEASE=' + args.library,
                       '-DFREETYPE_INCLUDE_DIR_freetype2=' + args.include,
                       '-DFREETYPE_INCLUDE_DIR_ft2build=' + args.include]
            result = subprocess.run(command, capture_output=True, text=True, timeout=30)
            output = result.stdout + result.stderr
            if (diagnostic is None and result.returncode != 0) or (diagnostic is not None and
                    (result.returncode == 0 or diagnostic not in output)):
                raise RuntimeError('Unexpected configuration result for {}: {}\n{}'.format(case, result.returncode, output))
            print('Font configuration {}: PASS'.format(case), flush=True)


if __name__ == '__main__':
    main()
