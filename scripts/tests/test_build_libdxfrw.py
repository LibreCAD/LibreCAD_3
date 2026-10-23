"""Synthetic provider-boundary checks, not native compiler or DXF evidence."""

import argparse
import copy
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import build_libdxfrw as builder


class LibdxfrwProviderTests(unittest.TestCase):
    def setUp(self):
        temporary=tempfile.TemporaryDirectory(prefix='dxfrw-provider-fixture-');self.addCleanup(temporary.cleanup)
        self.root=Path(temporary.name).resolve();self.source=self.root/builder.SUBMODULE;self.source.mkdir(parents=True)
        (self.source/'.git').write_text('gitdir: synthetic, never used\n')
        (self.source/'COPYING').write_text('synthetic license fixture\n')
        (self.source/'CMakeLists.txt').write_text('# synthetic upstream input\n')
        (self.root/'scripts').mkdir();(self.root/'patches').mkdir()
        self.revision='a'*40
        (self.root/'scripts/patch-libdxfrw.sh').write_text('expected_revision='+self.revision+'\n')
        (self.root/'scripts/renderer_process.py').write_text('# synthetic helper\n')
        (self.root/'patches/libdxfrw-required.patch').write_text('synthetic required patch\n')
        self.entry='160000 '+self.revision+' 0\t'+builder.SUBMODULE;self.actual=self.revision;self.dirty=''
        self.args=argparse.Namespace(platform='MAC-ARM64',report_dir=self.root/'evidence',cmake=Path(sys.executable),generator=None,timeout=30)
        self.commands=[]
        for target,value in (('REPO_ROOT',self.root),('git',self.git),('native_platform',lambda:self.args.platform)):
            patched=patch.object(builder,target,value);patched.start();self.addCleanup(patched.stop)

    def git(self,*args):
        if args[0]=='ls-files':return self.entry
        if args[-2:]==('rev-parse','HEAD'):return self.actual
        if 'status' in args:return self.dirty
        if args[-2:]==('ls-files','-z'):return 'COPYING\0CMakeLists.txt'
        self.fail('Unexpected synthetic Git query: '+str(args))

    def command(self,command,where,timeout,env):
        self.commands.append((command,where,env));self.assertLessEqual(timeout,self.args.timeout)
        source=where.parent/'source'
        self.assertEqual(str(source.parent),env.get('GIT_CEILING_DIRECTORIES'))
        for key in env:self.assertFalse(key.startswith('GIT_') and key!='GIT_CEILING_DIRECTORIES')
        self.assertNotIn('DESTDIR',env)
        if where.name.endswith('-apply'):(source/'CMakeLists.txt').write_text('# patched synthetic input\n')
        if where.name=='install':
            prefix=where.parent/'sdk';include=prefix/'include/libdxfrw';include.mkdir(parents=True)
            (include/'libdxfrw.h').write_text('// synthetic SDK header\n')
            config=prefix/'lib/cmake/libdxfrw';config.mkdir(parents=True)
            (config/'libdxfrwConfig.cmake').write_text('# synthetic SDK config\n')
            (prefix/('lib/dxfrw.lib' if self.args.platform=='WIN-X64' else 'lib/libdxfrw.a')).write_bytes(b'synthetic-not-native-archive')
        return dict(synthetic_only=True)

    def execute(self):
        with patch.object(builder,'run_command',side_effect=self.command):return builder.build(self.args)

    def test_all_platform_routes_build_only_the_patched_pic_library_in_owned_prefix(self):
        for platform in ('MAC-ARM64','MAC-X64','LINUX-ARM64','LINUX-X64','WIN-X64'):
            self.args.platform=platform;self.commands=[];original=builder.source_identity()
            with self.subTest(platform=platform),patch.dict(os.environ,GIT_DIR='/foreign',DESTDIR='/foreign'):
                directory=self.execute()
            record=json.loads((directory/'provider.json').read_text())
            self.assertEqual('static-pic',record['linkage']);self.assertFalse(record['phase_or_platform_accepted'])
            self.assertEqual(original,builder.source_identity());self.assertEqual(5,len(self.commands))
            configure=self.commands[2][0]
            for flag in ('-DBUILD_SHARED_LIBS=OFF','-DCMAKE_POSITION_INDEPENDENT_CODE=ON',
                    '-DLIBDXFRW_BUILD_DOC=OFF','-DLIBDXFRW_BUILD_DWG2DXF=OFF'):self.assertIn(flag,configure)
            self.assertNotEqual(record['source_sha256']['CMakeLists.txt'],record['patched_source_sha256']['CMakeLists.txt'])
            self.assertEqual(original['source_sha256']['COPYING'],record['sdk_sha256']['share/licenses/libdxfrw/COPYING'])
            for command,_,_ in self.commands:self.assertNotIn('sudo',command);self.assertNotIn('clone',command)

    def test_missing_wrong_dirty_and_mismatched_patch_revision_sources_fail_before_build(self):
        for mode in ('missing','wrong','dirty','patch'):
            with self.subTest(mode=mode):
                entry=self.entry;actual=self.actual;dirty=self.dirty;patcher=self.root/'scripts/patch-libdxfrw.sh';text=patcher.read_text()
                if mode=='missing':self.entry=''
                if mode=='wrong':self.actual='b'*40
                if mode=='dirty':self.dirty=' M CMakeLists.txt'
                if mode=='patch':patcher.write_text('expected_revision='+'b'*40+'\n')
                with self.assertRaises(ValueError):self.execute()
                self.entry=entry;self.actual=actual;self.dirty=dirty;patcher.write_text(text)
        self.assertEqual([],self.commands)

    def test_source_symlinks_and_missing_patch_are_not_accepted(self):
        file=self.source/'CMakeLists.txt';file.unlink();file.symlink_to(self.source/'COPYING')
        with self.assertRaisesRegex(ValueError,'source member'):builder.source_identity()
        file.unlink();file.write_text('# source\n');(self.root/'patches/libdxfrw-required.patch').unlink()
        with self.assertRaisesRegex(ValueError,'required patches'):builder.source_identity()

    def test_source_drift_or_missing_sdk_never_publishes_provider_success(self):
        original=self.command
        for mode in ('drift','missing'):
            def changed(command,where,timeout,env):
                result=original(command,where,timeout,env)
                if where.name=='install':
                    if mode=='drift':(where.parent/'source/CMakeLists.txt').write_text('# foreign mutation\n')
                    else:(where.parent/'sdk/lib/libdxfrw.a').unlink()
                return result
            with self.subTest(mode=mode),patch.object(builder,'run_command',side_effect=changed),self.assertRaises(ValueError):builder.build(self.args)
        self.assertEqual([],list(self.args.report_dir.rglob('provider.json')))

    def test_foreign_native_tuple_is_rejected_before_copying_sources(self):
        with patch.object(builder,'native_platform',return_value='OTHER'),self.assertRaisesRegex(ValueError,'matching native'):self.execute()
        self.assertFalse(self.args.report_dir.exists())

    def test_silent_patch_skip_never_publishes_an_unpatched_sdk(self):
        with patch.object(builder,'run_command',return_value=dict(synthetic_only=True)):
            with self.assertRaisesRegex(ValueError,'did not modify'):builder.build(self.args)
        self.assertEqual([],list(self.args.report_dir.rglob('provider.json')))

    @unittest.skipUnless(shutil.which('git'),'Git patch isolation regression requires Git')
    def test_git_discovery_floor_patches_the_staged_tree_inside_parent_checkout(self):
        subprocess.run(['git','init','--quiet',str(self.root)],check=True,timeout=10)
        nested=self.root/'evidence/source';nested.mkdir(parents=True)
        (nested/'file.txt').write_text('before\n')
        patchfile=self.root/'fixture.patch'
        patchfile.write_text('diff --git a/file.txt b/file.txt\n--- a/file.txt\n+++ b/file.txt\n@@ -1 +1 @@\n-before\n+after\n')
        env={k:v for k,v in os.environ.items() if not k.startswith('GIT_')}
        env['GIT_CEILING_DIRECTORIES']=str(nested.parent)
        subprocess.run(['git','-C',str(nested),'apply','--check',str(patchfile)],env=env,check=True,timeout=10)
        subprocess.run(['git','-C',str(nested),'apply',str(patchfile)],env=env,check=True,timeout=10)
        self.assertEqual('after\n',(nested/'file.txt').read_text())
        self.assertFalse((self.root/'file.txt').exists())


if __name__=='__main__':unittest.main()
