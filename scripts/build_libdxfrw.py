#!/usr/bin/env python3
"""Build the pinned source submodule into an isolated SDK; never install globally."""

import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time

from renderer_process import run_command
from run_renderer_checks import native_platform
from validate_renderer_inventory import REPO_ROOT, sha256


SUBMODULE='third_party/libdxfrw'


def git(*args):
    env={k:v for k,v in os.environ.items() if not k.startswith('GIT_')}
    return subprocess.check_output(['git',*args],cwd=REPO_ROOT,env=env,timeout=30).decode().strip()


def source_identity():
    """Reject missing, dirty or wrong-revision sources before invoking CMake."""
    entry=git('ls-files','--stage','--',SUBMODULE)
    match=re.fullmatch(r'160000 ([0-9a-f]{40}) 0\t'+re.escape(SUBMODULE),entry)
    source=REPO_ROOT/SUBMODULE
    if not match or not (source/'.git').is_file():
        raise ValueError('Initialize the pinned third_party/libdxfrw submodule first')
    revision=match.group(1)
    if git('-C',str(source),'rev-parse','HEAD')!=revision or git('-C',str(source),'status','--porcelain','--untracked-files=all'):
        raise ValueError('libdxfrw submodule must be clean at its recorded gitlink')
    patcher=(REPO_ROOT/'scripts/patch-libdxfrw.sh').read_text()
    if 'expected_revision='+revision+'\n' not in patcher:
        raise ValueError('libdxfrw submodule and required patch revision disagree')
    paths=git('-C',str(source),'ls-files','-z').rstrip('\0').split('\0')
    files={}
    for name in paths:
        path=source/name
        if Path(name).is_absolute() or '..' in Path(name).parts or path.is_symlink() or not path.is_file():
            raise ValueError('Unsupported libdxfrw source member')
        files[name]=sha256(path.read_bytes())
    patches=sorted((REPO_ROOT/'patches').glob('libdxfrw-*.patch'))
    if not files or not patches:raise ValueError('Missing libdxfrw sources or required patches')
    return dict(revision=revision,source_sha256=files,patches_sha256={p.name:sha256(p.read_bytes()) for p in patches},
        patcher_sha256=sha256(patcher.encode()))


def build(args):
    if args.platform!=native_platform():raise ValueError('libdxfrw requires the matching native platform')
    identity=source_identity();args.report_dir.mkdir(parents=True,exist_ok=True)
    directory=Path(tempfile.mkdtemp(prefix='dxfrw-',dir=args.report_dir)).resolve()
    source=directory/'source';source.mkdir();binary=directory/'build';prefix=directory/'sdk'
    for name in identity['source_sha256']:
        target=source/name;target.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(REPO_ROOT/SUBMODULE/name,target)
    env={k:v for k,v in os.environ.items() if not k.startswith('GIT_') and k!='DESTDIR'}
    commands=[];deadline=time.monotonic()+600
    def step(command,label):
        where=directory/label;where.mkdir();remaining=deadline-time.monotonic()
        if remaining<=0:raise ValueError('libdxfrw provider deadline expired')
        commands.append(run_command(command,where,min(args.timeout,remaining),env))
    git_executable=str(Path(shutil.which('git')).resolve(strict=True))
    for name in identity['patches_sha256']:
        patch=REPO_ROOT/'patches'/name
        step([git_executable,'-C',str(source),'apply','--check',str(patch)],name+'-check')
        step([git_executable,'-C',str(source),'apply',str(patch)],name+'-apply')
    patched={name:sha256((source/name).read_bytes()) for name in identity['source_sha256']}
    cmake=str(Path(args.cmake or shutil.which('cmake')).resolve(strict=True))
    command=[cmake,'-S',str(source),'-B',str(binary),'-DCMAKE_BUILD_TYPE=Release',
        '-DCMAKE_CONFIGURATION_TYPES=Release','-DCMAKE_POLICY_VERSION_MINIMUM=3.5',
        '-DCMAKE_INSTALL_PREFIX='+str(prefix),'-DCMAKE_INSTALL_LIBDIR=lib','-DBUILD_SHARED_LIBS=OFF','-DCMAKE_POSITION_INDEPENDENT_CODE=ON',
        '-DLIBDXFRW_BUILD_DOC=OFF','-DLIBDXFRW_BUILD_DWG2DXF=OFF']
    if args.generator:command+=['-G',args.generator]
    if args.platform.startswith('LINUX-'):command+=['-DCMAKE_CXX_FLAGS=-Wno-error=misleading-indentation']
    step(command,'configure')
    step([cmake,'--build',str(binary),'--config','Release','--parallel','3'],'compile')
    step([cmake,'--install',str(binary),'--config','Release'],'install')
    notice=prefix/'share/licenses/libdxfrw/COPYING';notice.parent.mkdir(parents=True)
    shutil.copy2(source/'COPYING',notice)
    # A PIC archive links into persistence without an unshipped DXF runtime or
    # an SDK-specific loader search path in installed project binaries.
    library=prefix/('lib/dxfrw.lib' if args.platform=='WIN-X64' else 'lib/libdxfrw.a')
    include=prefix/'include/libdxfrw';config=prefix/'lib/cmake/libdxfrw'
    for path in (library,include/'libdxfrw.h',config/'libdxfrwConfig.cmake',notice):
        if not path.is_file() or not path.resolve().is_relative_to(prefix):raise ValueError('Missing/escaping built libdxfrw SDK file')
    if identity!=source_identity() or patched!={name:sha256((source/name).read_bytes()) for name in patched}:
        raise ValueError('libdxfrw provider source changed during execution')
    outputs={p.relative_to(prefix).as_posix():sha256(p.read_bytes()) for p in prefix.rglob('*') if p.is_file()}
    record=dict(schema_version=1,evidence_kind='libdxfrw-submodule-sdk',platform=args.platform,
        phase_or_platform_accepted=False,linkage='static-pic',**identity,patched_source_sha256=patched,
        builder_sha256=sha256(Path(__file__).read_bytes()),
        process_helper_sha256=sha256((REPO_ROOT/'scripts/renderer_process.py').read_bytes()),
        sdk_sha256=outputs,commands=commands,profile_budget_seconds=600,command_timeout_seconds=args.timeout,
        cmake_options=dict(LIBDXFRW_INCLUDE_DIR=str(include),LIBDXFRW_LIBRARY=str(library.resolve()),libdxfrw_DIR=str(config)))
    (directory/'provider.json').write_text(json.dumps(record,indent=2)+'\n')
    print(json.dumps(dict(provider=str(directory/'provider.json'),sdk=str(prefix),cmake_options=record['cmake_options'])))
    return directory


def main():
    parser=argparse.ArgumentParser(description=__doc__,allow_abbrev=False)
    parser.add_argument('--platform',required=True,choices=('MAC-ARM64','MAC-X64','LINUX-ARM64','LINUX-X64','WIN-X64'))
    parser.add_argument('--report-dir',required=True,type=Path)
    parser.add_argument('--cmake',type=Path)
    parser.add_argument('--generator')
    parser.add_argument('--timeout',type=float,default=180)
    args=parser.parse_args()
    if not 0<args.timeout<=300:parser.error('timeout must be positive and at most 300 seconds')
    try:build(args)
    except (ValueError,OSError,subprocess.SubprocessError) as error:parser.exit(1,'libdxfrw SDK: FAIL: '+str(error)+'\n')


if __name__=='__main__':main()
