"""Synthetic source orchestration/workflow checks, never native build/GPU proof."""
import argparse
import copy
from contextlib import ExitStack
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import run_renderer_checks as runner
import validate_renderer_inventory as inventory


class KernelObjectTests(unittest.TestCase):
    def setUp(self):
        temporary=tempfile.TemporaryDirectory(prefix='renderer-kernel-objects-');self.addCleanup(temporary.cleanup)
        self.root=Path(temporary.name).resolve();self.build=self.root/'build';self.build.mkdir()
        self.target=dict(paths=dict(build='lckernel'),compileGroups=[dict(language='CXX')],sources=[])
        self.entries=[]
        for index in range(65):
            source=self.root/('lckernel/source'+str(index)+'.cpp')
            self.target['sources'].append(dict(path=str(source),compileGroupIndex=0))
            self.entries.append(dict(directory=str(self.build),file=str(source),
                output=str(self.build/('lckernel/CMakeFiles/lckernel.dir/source'+str(index)+'.cpp.o'))))
        self.write()
        self.root_patch=patch.object(inventory,'REPO_ROOT',self.root);self.root_patch.start();self.addCleanup(self.root_patch.stop)
        self.real_graph=runner.source_targets
        self.graph_patch=patch.object(runner,'source_targets',side_effect=lambda _:dict(lckernel=self.target))
        self.graph_patch.start();self.addCleanup(self.graph_patch.stop)

    def write(self):
        (self.build/'compile_commands.json').write_text(json.dumps(self.entries))

    def test_actual_source_inventory_maps_to_unique_owned_objects(self):
        self.entries.append(dict(directory=str(self.build),file=str(self.root/'other.cpp'),
            output=str(self.build/'other/CMakeFiles/other.dir/other.cpp.o')));self.write()
        objects=runner.kernel_object_targets(self.build)
        self.assertEqual(65,len(objects));self.assertEqual(65,len(set(objects)))
        self.assertTrue(all(path.startswith('lckernel/CMakeFiles/lckernel.dir/') for path in objects))

    def test_private_cache_requires_its_exact_owned_source_output_pair(self):
        owner=self.build/'lckernel/CMakeFiles/lckernel.dir'
        source=owner/'cmake_pch.hxx.cxx';output=owner/'cmake_pch.hxx.pch'
        self.target['sources'].append(dict(path=str(source),compileGroupIndex=0))
        self.entries.append(dict(directory=str(self.build),file=str(source),output=str(output)));self.write()
        self.assertEqual(66,len(runner.kernel_object_targets(self.build)))
        self.assertIn(str(output.relative_to(self.build)),runner.kernel_object_targets(self.build))
        original=copy.deepcopy(self.entries)
        for mutate in (lambda rows:rows.pop(),lambda rows:rows.append(rows[-1]),
                lambda rows:rows[-1].update(output=str(owner/'cmake_pch.hxx.cxx.o')),
                lambda rows:rows[-1].update(output=str(owner/'foreign.pch')),
                lambda rows:rows[-1].update(file=str(owner/'foreign.cxx')),
                lambda rows:rows[0].update(output=str(output)),
                lambda rows:rows[-1].update(output=str(owner/'../cmake_pch.hxx.pch'))):
            self.entries=copy.deepcopy(original);mutate(self.entries);self.write()
            with self.subTest(mutate=mutate),self.assertRaises(ValueError):runner.kernel_object_targets(self.build)

    def test_real_ninja_pch_graph_maps_and_builds_every_compiled_kernel_input(self):
        kernel=self.root/'lckernel';kernel.mkdir()
        (kernel/'tiny.cpp').write_text('int fixture() { return 7; }\n')
        (kernel/'cache.hpp').write_text('#pragma once\n#include <vector>\n')
        (kernel/'CMakeLists.txt').write_text('add_library(lckernel SHARED tiny.cpp)\n'
            'target_precompile_headers(lckernel PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cache.hpp")\n')
        (self.root/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.28)\nproject(KernelCache LANGUAGES CXX)\n'
            'set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\nadd_subdirectory(lckernel)\n')
        query=self.build/'.cmake/api/v1/query/client-renderer-source';query.mkdir(parents=True)
        (query/'codemodel-v2').touch()
        command=['cmake','-S',str(self.root),'-B',str(self.build),'-G','Ninja','-DCMAKE_BUILD_TYPE=Release']
        result=subprocess.run(command,capture_output=True,text=True,timeout=30)
        self.assertEqual(0,result.returncode,result.stdout+result.stderr)
        with patch.object(runner,'source_targets',self.real_graph):
            if sys.platform=='win32':
                # Chunking is Intel Mac/Ninja only; MSVC .obj mappings remain rejected.
                with self.assertRaises(ValueError):runner.kernel_object_targets(self.build)
                return
            outputs=runner.kernel_object_targets(self.build)
        self.assertEqual(2,len(outputs));self.assertEqual(1,sum(p.endswith(('.pch','.gch')) for p in outputs))
        result=subprocess.run(['cmake','--build',str(self.build),'--target',*outputs,'--parallel','4'],
            capture_output=True,text=True,timeout=30)
        self.assertEqual(0,result.returncode,result.stdout+result.stderr)
        self.assertTrue(all((self.build/path).is_file() for path in outputs))

    def test_missing_duplicate_foreign_and_escaping_mapping_is_rejected(self):
        original=copy.deepcopy(self.entries)
        mutations=[lambda rows:rows.pop(),lambda rows:rows.append(rows[0]),
            lambda rows:rows[0].update(output='/foreign/kernel.o'),
            lambda rows:rows[0].update(output=str(self.build/'lckernel/CMakeFiles/lckernel.dir/../escape.o')),
            lambda rows:rows[0].update(output=rows[1]['output']),
            lambda rows:rows[0].update(file=str(self.root/'foreign.cpp')),
            lambda rows:rows[0].update(directory=str(self.root)),
            lambda rows:rows[0].pop('output')]
        for mutate in mutations:
            self.entries=copy.deepcopy(original);mutate(self.entries);self.write()
            with self.subTest(mutate=mutate),self.assertRaises(ValueError):runner.kernel_object_targets(self.build)
        self.entries=original;self.write()
        for change in (lambda: self.target['sources'].append(self.target['sources'][0]),
                       lambda: self.target['sources'][0].update(compileGroupIndex=1)):
            before=copy.deepcopy(self.target);change()
            with self.assertRaises(ValueError):runner.kernel_object_targets(self.build)
            self.target=before

    def test_intel_ninja_prebuild_has_bounded_chunks_and_other_generators_are_unchanged(self):
        cache=self.build/'CMakeCache.txt';cache.write_text('CMAKE_GENERATOR:INTERNAL=Ninja\n')
        calls=[]
        with patch.object(sys,'platform','darwin'),patch.object(runner.platform,'machine',return_value='x86_64'):
            runner.prebuild_cold_kernel('/cmake',self.build,lambda command,name:calls.append((command,name)))
        self.assertEqual(['kernel-part-1','kernel-part-2','kernel-part-3'],[name for _,name in calls])
        targets=[command[command.index('--target')+1:command.index('--parallel')] for command,_ in calls]
        self.assertEqual([32,32,1],list(map(len,targets)))
        self.assertEqual(runner.kernel_object_targets(self.build),[target for chunk in targets for target in chunk])
        for platform,generator in (('MAC-ARM64','Ninja'),('MAC-X64','Unix Makefiles'),('WIN-X64','Visual Studio 17 2022')):
            cache.write_text('CMAKE_GENERATOR:INTERNAL='+generator+'\n')
            with patch.object(sys,'platform','darwin' if platform.startswith('MAC-') else 'win32'), \
                    patch.object(runner.platform,'machine',return_value='arm64' if platform.endswith('ARM64') else 'x86_64'), \
                    patch.object(runner,'kernel_object_targets') as read:
                runner.prebuild_cold_kernel('/cmake',self.build,lambda *_:self.fail('Unexpected prebuild'))
                read.assert_not_called()


class SourceBuildTests(unittest.TestCase):
    def test_build_metadata_stays_stable_per_profile_but_real_inputs_still_refresh(self):
        with tempfile.TemporaryDirectory(prefix='renderer-build-metadata-') as temporary:
            root=Path(temporary);kernel=root/'lckernel';kernel.mkdir()
            shutil.copyfile(inventory.REPO_ROOT/'lckernel/build_constants.cpp.in',kernel/'build_constants.cpp.in')
            def run(argv,env=None):
                result=subprocess.run(argv,cwd=root,env=env,capture_output=True,text=True,timeout=30)
                self.assertEqual(0,result.returncode,result.stdout+result.stderr)
                return result.stdout.strip()
            run(['git','init','-q'])
            commit=['git','-c','user.name=Fixture','-c','user.email=fixture@example.invalid',
                    '-c','commit.gpgsign=false','commit','--allow-empty','-qm']
            run([*commit,'first'])
            output=root/'build_constants.cpp'
            command=['cmake','-DDIR='+str(kernel),'-DOUTPUT_FILE='+str(output),'-DLIBDXFRW_BUILD_VERSION=fixture',
                     '-P',str(inventory.REPO_ROOT/'lckernel/cmake/SetBuildInfo.cmake')]
            env=dict(os.environ,SOURCE_DATE_EPOCH='1700000000')
            run(command,env);first=output.read_bytes();os.utime(output,ns=(1_000_000_000,1_000_000_000))
            run(command,env)
            self.assertEqual(first,output.read_bytes());self.assertEqual(1_000_000_000,output.stat().st_mtime_ns)
            env['SOURCE_DATE_EPOCH']='1700000001';run(command,env);second=output.read_bytes()
            self.assertNotEqual(first,second);self.assertNotEqual(1_000_000_000,output.stat().st_mtime_ns)
            os.utime(output,ns=(1_000_000_000,1_000_000_000))
            run([*commit,'second']);revision=run(['git','rev-parse','--short','HEAD'])
            run(command,env)
            self.assertNotEqual(second,output.read_bytes());self.assertIn(('commit '+revision).encode(),output.read_bytes())
            self.assertNotEqual(1_000_000_000,output.stat().st_mtime_ns)

    def test_r2_3_actual_regions_share_curve_recording_and_propagate_join_certificates(self):
        root=inventory.REPO_ROOT
        source=(root/'unittest/lcviewernoqt/testregionrecorder.cpp').read_text()
        names=set(re.findall(r'TEST\(CpuRegionRecorderTest,\s*(\w+)\)',source))
        self.assertEqual(18,len(names))
        self.assertIn('EllipseAxesValidateBeforeAnyRegionBoundaryOrPreparedInkEscapes',names)
        self.assertIn('AnalyticEndpointClosureIsPanInvariantButRealGapsStillReject',names)
        self.assertIn('lcviewernoqt/testregionrecorder.cpp',(root/'unittest/CMakeLists.txt').read_text().split('# Issue #412')[0])
        production=(root/'lcviewernoqt/painters/hatchrecorder.cpp').read_text()
        for name in ('recordRegion','prepareRegionFill','prepareRegionHatch','ambiguous-region-loop','region-work-limit','input.positionBound'):
            self.assertIn(name,production)
        self.assertIn('flat.positionBound+inputPositionBound',(root/'lcviewernoqt/painters/pathfill.cpp').read_text())
        self.assertIn('nullptr,inputPositionBound',(root/'lcviewernoqt/painters/hatchtiler.cpp').read_text())

    def test_r2_3_real_tile_preparation_is_bounded_cpu_geometry_in_the_ordinary_suite(self):
        root=inventory.REPO_ROOT
        self.assertIn('painters/hatchtiler.cpp',(root/'lcviewernoqt/CMakeLists.txt').read_text())
        self.assertIn('lcviewernoqt/testhatchtiler.cpp',(root/'unittest/CMakeLists.txt').read_text().split('# Issue #412')[0])
        source=(root/'unittest/lcviewernoqt/testhatchtiler.cpp').read_text()
        self.assertEqual(13,len(set(re.findall(r'TEST\(CpuHatchTilerTest,\s*(\w+)\)',source))))
        for name in ('LargeWorldFractionalPeriodPhaseMatchesIndependentBinaryMultiprecision',
                     'DashPhaseStaysOnWholeFinitePrimitivesAcrossMaskAndTileBoundaries',
                     'GlobalCellsInstancesVerticesAndWorkCapsFailAtomicallyAndRecover'):
            self.assertIn(name,source)
        production=(root/'lcviewernoqt/painters/hatchtiler.cpp').read_text()
        for api in ('prepareFill','prepareStroke','hatch-cell-limit','hatch-instance-limit','hatch-work-limit'):
            self.assertIn(api,production)
        self.assertIn('#include "pathnumeric_p.h"',production)
        self.assertIn('using detail::Sum;',production)
        self.assertIn('std::fma',(root/'lcviewernoqt/painters/pathnumeric_p.h').read_text())
        for forbidden in ('#include <Q','#include <cairo','QRhi','glDraw','multiprecision','long double'):
            self.assertNotIn(forbidden,production)

    def test_r2_3_real_cad_tile_recording_stays_owned_bounded_and_in_the_ordinary_suite(self):
        root=inventory.REPO_ROOT
        source=(root/'unittest/lcviewernoqt/testhatchrecorder.cpp').read_text()
        names=set(re.findall(r'TEST\(CpuHatchRecorderTest,\s*(\w+)\)',source))
        self.assertEqual(11,len(names))
        for name in ('RealProviderGeometryTypesKeepOrderAndOwnedModelCoordinates',
                     'EllipseAxesArePlanarFiniteAndFailAtomicallyBeforeProjection',
                     'ConnectedSplineSpansKeepWholePrimitiveDashPhaseAndOnlyEndpointCaps',
                     'DisconnectedSplineProviderSpansFailWithoutInventedBridgesOrPartialCommands',
                     'AggregateRecorderCapsFailAtomicallyAndRecover',
                     'InvalidNullUnknownAndNonplanarEntitiesCannotDisappearSilently'):
            self.assertIn(name,names)
        self.assertIn('lcviewernoqt/testhatchrecorder.cpp',(root/'unittest/CMakeLists.txt').read_text().split('# Issue #412')[0])
        self.assertIn('painters/hatchrecorder.cpp',(root/'lcviewernoqt/CMakeLists.txt').read_text())
        production=(root/'lcviewernoqt/painters/hatchrecorder.cpp').read_text()
        self.assertIn('bezierCount()',production);self.assertIn('bezierAt(i)',production)
        self.assertNotIn('->beziers()',production)
        self.assertIn('recorder.commands()',production)
        for forbidden in ('#include <Q','#include <cairo','glDraw','QRhi'):
            self.assertNotIn(forbidden,production)

    def test_r2_3_fill_bindings_execute_real_bounded_cpu_geometry(self):
        root=inventory.REPO_ROOT
        manifest=inventory.validate_case_manifest()['manifest']
        case=next(c for c in manifest['cases'] if c['id']=='cpu-hatch-fill')
        source=(root/'unittest/lcviewernoqt/testpathrecorder.cpp').read_text()
        implemented={suite+'.'+name for suite,name in re.findall(r'TEST\((\w+),\s*(\w+)\)',source)}
        self.assertEqual(6,len(case['variants']))
        self.assertTrue({v['test'] for v in case['variants']}<=implemented)
        self.assertIn('painters/pathfill.cpp',(root/'lcviewernoqt/CMakeLists.txt').read_text())
        for name in ('WindingOrientationNestingAndDisconnectedRegions','TopologyCrossingTouchingAndRetracingRejectAtomically',
                     'CurvedCircleAndPolynomialBoundaryHaveIndependentAreaAndDistance',
                     'WorkVertexStateAndBatchCapsPreserveOrderAndPriorOutput',
                     'EllipsesUseLocalCurveCertificatesAndRetainAffineAreaAndMembership',
                     'ScreenSizedCircleUsesBoundedSweepWorkAndPreservesLimits',
                     'AdjacentCurvesCannotHideCrossingsBehindSharedEndpoints',
                     'CurveRefinementChargesDiscardedAttemptsWithinTheOriginalBatchBudget'):
            self.assertIn('CpuFillTest.'+name,implemented)
        self.assertIn('CpuHatchTest.FiniteWidthCoverageUsesMaskNotCenterlineClipping',implemented)
        self.assertIn('CpuGradientTest.StableDuplicatesStraightAlphaPaddingAndOwnedSnapshot',implemented)
        production=(root/'lcviewernoqt/painters/pathfill.cpp').read_text()
        for forbidden in ('#include <Q','#include <cairo','multiprecision','long double'):
            self.assertNotIn(forbidden,production)
        for api in ('prepareFill','prepareFills','prepareHatch','prepareLinearGradient'):
            self.assertIn(api,(root/'lcviewernoqt/painters/pathgeometry.h').read_text())

    def test_r2_2_owner_selected_points_and_continuity_keep_ordinary_regressions(self):
        source=(inventory.REPO_ROOT/'unittest/lcviewernoqt/testpathrecorder.cpp').read_text()
        for name in ('PhysicalAndModelDiameterFollowIndependentAffineLandmarks',
                     'MarksOwnStatePreservePathAndBatchOrderWithoutStrokeDecorations',
                     'AggregateWorkVerticesAndPhysicalMajorAxisLimitsFailClosed',
                     'CorrectedInnerFanClosesShortCornersWithoutUnboundedMiters',
                     'InternalCuspContinuityDoesNotDecorateExplicitReversals',
                     'DashedCuspKeepsContinuityOnlyInsideAnOnRun'):
            self.assertRegex(source,r'TEST\(\w+,\s*'+name+r'\)')

    def test_r2_2_monotone_certificates_keep_exact_and_ambiguous_domains_separate(self):
        source=(inventory.REPO_ROOT/'unittest/lcviewernoqt/testpathrecorder.cpp').read_text()
        tests=set(re.findall(r'TEST\(CpuCurvedStrokeTest,\s*(\w+)\)',source))
        for name in ('MonotoneControlPolygonsDoNotNeedNearCriticalDiscriminantRoots',
                     'StrictlyMonotoneNearStationaryCubicsUseAnExactSignCertificate',
                     'FmaResidualOrderingCertifiesAxisAlignedOffsetsWithoutHidingReversals',
                     'NearCriticalScalarRoundingFailsClosedButExactStationarySpansWork'):
            self.assertIn(name,tests)
        production=(inventory.REPO_ROOT/'lcviewernoqt/painters/pathgeometry.cpp').read_text()
        self.assertNotIn('long double',production.replace('long-double',''))
        self.assertNotIn('multiprecision',production)

    def test_r2_2_curve_work_caps_cover_nonrecursive_and_mixed_paths(self):
        source=(inventory.REPO_ROOT/'unittest/lcviewernoqt/testpathrecorder.cpp').read_text()
        tests=set(re.findall(r'TEST\(CpuCurveTest,\s*(\w+)\)',source))
        for name in ('WorkCapCountsMoveLineCloseAndEmptySubpaths','MixedCurveWorkSharesTheCapAndFailurePreservesPriorOutput'):
            self.assertIn(name,tests)
        production=(inventory.REPO_ROOT/'lcviewernoqt/painters/pathgeometry.cpp').read_text()
        self.assertIn('command.path.size() <= limits.work',production)

    def test_r2_2_shared_geometry_bindings_and_batch_stay_cpu_only(self):
        cases=inventory.validate_case_manifest()
        names={'line','arc','circle','ellipse','lwpolyline','stroke','point'}
        by_id={c['id']:c for c in cases['manifest']['cases']}
        source=(inventory.REPO_ROOT/'unittest/lcviewernoqt/testpathrecorder.cpp').read_text()
        implemented={suite+'.'+name for suite,name in re.findall(r'TEST\((\w+),\s*(\w+)\)',source)}
        count=0
        for name in names:
            gpu,cpu=by_id[name],by_id['cpu-'+name]
            for key in ('dataset','fixture','features','methods','consumers','assets','decisions','stage'):
                self.assertEqual(gpu[key],cpu[key])
            self.assertEqual(['cpu-source'],cpu['configurations']);self.assertEqual(['cpu'],cpu['profiles'])
            self.assertEqual(['cpu'],cpu['modes']);self.assertEqual(['none'],cpu['devices']);self.assertEqual([],cpu['requires'])
            for g,c in zip(gpu['variants'],cpu['variants']):
                self.assertEqual({k:v for k,v in g.items() if k!='test'},{k:v for k,v in c.items() if k!='test'})
                self.assertIn(c['test'],implemented);self.assertNotEqual(g['test'],c['test']);count+=1
        self.assertEqual(26,count)
        for name in ('BatchOwnsSnapshotsIncludingEmptyCommands','BatchSharesWorkVerticesAndInputStateBudgets',
                     'BatchLaterInvalidCommandLeavesPriorOutputIntact'):
            self.assertIn('CpuStrokeTest.'+name,implemented)
        self.assertIn('prepareStrokes',(inventory.REPO_ROOT/'lcviewernoqt/painters/pathgeometry.h').read_text())

    def test_r2_2_curved_stroke_bounds_and_stationary_cases_use_the_cpu_suite(self):
        source=(inventory.REPO_ROOT/'unittest/lcviewernoqt/testpathrecorder.cpp').read_text()
        tests=set(re.findall(r'TEST\(CpuCurvedStrokeTest,\s*(\w+)\)',source))
        self.assertGreaterEqual(len(tests),17)
        for name in ('PolynomialNormalSweepsUseWidthNotOnlyCenterlineError','ThickClosedCircleKeepsAnnulusAndFoldedInterior',
                     'PhysicalDashesFollowAnalyticCircularArcLength','StationaryTwoDimensionalCuspUsesOneSidedTangents',
                     'SharpCurveBoundaryUsesExactTangentsAtTheMiterThreshold'):
            self.assertIn(name,tests)
        self.assertIn('CurveMetrics',(inventory.REPO_ROOT/'lcviewernoqt/painters/pathgeometry.h').read_text())

    def test_r2_2_linear_stroke_preparation_stays_in_the_ordinary_cpu_suite(self):
        root=inventory.REPO_ROOT
        source=(root/'unittest/lcviewernoqt/testpathrecorder.cpp').read_text()
        tests=set(re.findall(r'TEST\(CpuStrokeTest,\s*(\w+)\)',source))
        self.assertGreaterEqual(len(tests),18)
        self.assertIn('RoundJoinDoesNotOverpaintNearbyButtCaps',tests)
        self.assertIn('RoundCapsDoNotOverpaintNearbyBevelJoins',tests)
        self.assertIn('ZeroGapRetainsJoinsRatherThanIntermediateCaps',tests)
        self.assertIn('painters/pathstroke.cpp',(root/'lcviewernoqt/CMakeLists.txt').read_text())
        self.assertIn('lcviewernoqt/testpathrecorder.cpp',(root/'unittest/CMakeLists.txt').read_text().split('# Issue #412')[0])
        self.assertIn('UnionBeforeOver',(root/'lcviewernoqt/painters/pathgeometry.h').read_text())

    def test_r2_2_cpu_spline_ids_use_the_ordinary_recorder_suite(self):
        cases=inventory.validate_case_manifest()
        required={v['test'] for c in cases['manifest']['cases'] if c['id']=='cpu-spline' for v in c['variants']}
        source=(inventory.REPO_ROOT/'unittest/lcviewernoqt/testpathrecorder.cpp').read_text()
        implemented={suite+'.'+name for suite,name in re.findall(r'TEST\((\w+),\s*(\w+)\)',source)}
        self.assertEqual(7,len(required));self.assertTrue(required<=implemented)
        self.assertIn('build_contract::pinnedFixture',source)
        self.assertIn('"spline"',source)
        self.assertIn('painters/pathgeometry.h',runner.CPU_PUBLIC_HEADERS)

    def test_r2_1_contract_ids_are_in_the_ordinary_cpu_suite(self):
        cases=inventory.validate_case_manifest()
        required={v['test'] for c in cases['manifest']['cases'] if c['id'] in ('cpu-path-state','cpu-transform') for v in c['variants']}
        source=(inventory.REPO_ROOT/'unittest/lcviewernoqt/testpathrecorder.cpp').read_text()
        implemented={suite+'.'+name for suite,name in re.findall(r'TEST\((\w+),\s*(\w+)\)',source)}
        self.assertEqual(9,len(required));self.assertTrue(required<=implemented)
        cmake=(inventory.REPO_ROOT/'unittest/CMakeLists.txt').read_text()
        self.assertIn('lcviewernoqt/testpathrecorder.cpp',cmake.split('# Issue #412')[0])
        self.assertIn('build_contract::pinnedFixture',source)
        self.assertIn('CpuTransformTest.AffineCancellationPreservesModelDetailBeforePreparation',implemented)
        self.assertIn('CpuTransformTest.AffineResidualCancellationRejectsBeforePublishingSmallPhysicalCoordinates',implemented)

    def test_provider_period_regression_is_an_owned_persistence_test(self):
        source=(inventory.REPO_ROOT/'unittest/persistence/hatchcatalogtest.cpp').read_text()
        names=set(re.findall(r'TEST\(CpuHatchCatalogTest,\s*(\w+)\)',source))
        self.assertEqual(4,len(names))
        self.assertIn('ClayKeepsItsOriginBasedCellDespiteTheEmptyInkMargin',names)
        cmake=(inventory.REPO_ROOT/'unittest/CMakeLists.txt').read_text()
        self.assertIn('persistence/hatchcatalogtest.cpp',cmake)

    def test_source_compiler_workers_use_available_cores_with_a_four_worker_cap(self):
        for count,wanted in ((None,'1'),(0,'1'),(1,'1'),(2,'2'),(3,'3'),(4,'4'),(64,'4')):
            with self.subTest(count=count),patch.object(runner.os,'cpu_count',return_value=count):
                self.assertEqual(wanted,runner.source_parallelism())

    def setUp(self):
        temporary=tempfile.TemporaryDirectory(prefix='rhi-source-fixture-');self.addCleanup(temporary.cleanup)
        self.root=Path(temporary.name)
        self.args=argparse.Namespace(app=None,build_dir=None,case_cmake_option=[],cmake=Path(sys.executable),
            cmake_option=[],report_dir=self.root,generator='Ninja Multi-Config',timeout=10)
        self.selected=inventory.select_case_requirements(inventory.validate_case_manifest(),'R1',
            'gui-less-macos/MAC-ARM64/rhi-offscreen/offscreen/cocoa/hardware')
        self.commands=[];self.noise=False;self.failure=False

    def command(self,command,where,timeout,env):
        self.commands.append((command,env));name=where.name
        if name=='configure':
            flags={v[2:].split(':BOOL=')[0]:v.endswith('=ON') for v in command if ':BOOL=' in v}
            library_graph(Path(command[command.index('-B')+1]),flags)
        if name=='compile' or name.startswith('compile-'):
            build=Path(command[2]);(build/'bin').mkdir(parents=True,exist_ok=True)
            targets=runner.source_targets(build)
            for target_name,target in targets.items():
                if 'paths' not in target:continue
                pch=build/target['paths']['build']/'CMakeFiles'/(target_name+'.dir')
                if (pch/'cmake_pch.hxx').is_file():(pch/'cmake_pch.hxx.pch').write_bytes(b'synthetic-private-pch-not-native')
            if 'lcunittest' in targets:
                (build/'bin'/('lcunittest'+('.exe' if sys.platform=='win32' else ''))).write_bytes(b'not-an-executable')
                for object_name in (build/'lcviewernoqt/cpu-headers/objects-Release.txt').read_text().splitlines():Path(object_name).write_bytes(b'synthetic-object-not-native')
            for target in targets.values():
                for item in target.get('artifacts',[]):
                    path=build/item['path'];path.parent.mkdir(exist_ok=True);path.write_bytes(b'synthetic-library-not-native')
        if name=='cpu-discovery':
            (where/'stdout.log').write_bytes((b'noise\n' if self.noise else b'LibreCAD v3.0 (synthetic fixture)\n')+b'S.\n  A\n  B\n')
        if name=='cpu-tests':
            root=ET.Element('testsuites',tests='2',failures='1' if self.failure else '0',disabled='0',errors='0')
            suite=ET.SubElement(root,'testsuite',name='S',tests='2',failures='0',disabled='0',errors='0',skipped='0')
            for n in ('A','B'):ET.SubElement(suite,'testcase',name=n,classname='S',status='run',result='completed')
            ET.ElementTree(root).write(where/'tests.xml')
        return dict(synthetic_only=True)

    def execute(self):
        with patch.object(runner,'rhi_selection',return_value=self.selected),patch.object(runner,'run_command',side_effect=self.command):
            return runner.build_rhi_source(self.args)

    def test_fresh_build_keeps_every_cpu_id_and_no_dependency_manager(self):
        with patch.dict(os.environ,GTEST_FILTER='S.A',SOURCE_DATE_EPOCH='poison'), \
                patch.object(runner.time,'time',return_value=1700000000.9):directory=self.execute()
        record=json.loads((directory/'source-build.json').read_text())
        self.assertIs(record['phase_or_platform_accepted'],False)
        self.assertEqual(['S.A','S.B'],record['cpu_executed_ids']);self.assertEqual(4,len(self.commands))
        for command,env in self.commands:
            self.assertNotIn('GTEST_FILTER',env);self.assertEqual('1700000000',env['SOURCE_DATE_EPOCH'])
        configure=self.commands[0][0];build=self.commands[1][0]
        for flag in ('WITH_QT_UI','WITH_PYTHONSCRIPT','WITH_LEGACY_OPENGL_RENDERER','WITH_PERSISTENCE'):
            self.assertIn('-D'+flag+':BOOL=OFF',configure)
        self.assertEqual(['lcrenderer_source_checks'],build[build.index('--target')+1:build.index('--parallel')])
        self.assertEqual(7,len(record['cpu_headers']['objects_sha256']))
        self.assertEqual(set(runner.SOURCE_PRIVATE_PCH),set(record['source_checks']['private_pch']))
        self.assertEqual(set(runner.SOURCE_PRIVATE_PCH),set(record['private_pch']))
        self.assertNotIn('kernel_pch',record);self.assertNotIn('kernel_pch',record['source_checks'])
        self.assertIn('--config',build);self.assertIn('Release',build)

    def test_aggregate_source_build_keeps_one_shared_300_second_deadline(self):
        self.args.timeout=300;observed=[]
        def command(argv,where,timeout,env):
            observed.append((where.name,timeout))
            return self.command(argv,where,timeout,env)
        with patch.object(runner.time,'monotonic',side_effect=[0,0,8.938,250,299.99]), \
                patch.object(runner,'rhi_selection',return_value=self.selected), \
                patch.object(runner,'run_command',side_effect=command):
            directory=runner.build_rhi_source(self.args)
        self.assertEqual(['configure','compile','cpu-discovery','cpu-tests'],[name for name,_ in observed])
        for (_,actual),expected in zip(observed,(300,291.062,50,.01)):
            self.assertAlmostEqual(expected,actual)
            self.assertTrue(0<actual<=300)
        record=json.loads((directory/'source-build.json').read_text())
        self.assertEqual(['S.A','S.B'],record['cpu_discovered_ids'])
        self.assertEqual(record['cpu_discovered_ids'],record['cpu_executed_ids'])
        self.assertEqual(7,len(record['cpu_headers']['objects_sha256']))

    def test_aggregate_compilation_cannot_restart_the_total_source_deadline(self):
        self.args.timeout=300;observed=[]
        def command(argv,where,timeout,env):
            observed.append((where.name,timeout))
            return self.command(argv,where,timeout,env)
        with patch.object(runner.time,'monotonic',side_effect=[0,0,8.938,300.001]), \
                patch.object(runner,'rhi_selection',return_value=self.selected), \
                patch.object(runner,'run_command',side_effect=command):
            with self.assertRaisesRegex(ValueError,'Source build deadline expired'):
                runner.build_rhi_source(self.args)
        self.assertEqual(['configure','compile'],[name for name,_ in observed])
        self.assertAlmostEqual(291.062,observed[-1][1])

    def test_final_source_aggregate_never_prebuilds_kernel_even_on_intel_ninja(self):
        with patch.object(sys,'platform','darwin'),patch.object(runner.platform,'machine',return_value='x86_64'), \
                patch.object(runner,'prebuild_cold_kernel') as prebuild:
            directory=self.execute()
        prebuild.assert_not_called()
        commands=json.loads((directory/'source-build.json').read_text())['commands']
        self.assertEqual(4,len(commands))

    def test_source_rejects_missing_aggregate_dependency_before_compilation(self):
        def command(argv,where,timeout,env):
            result=self.command(argv,where,timeout,env)
            if where.name=='configure':
                path=where.parent/'build/.cmake/api/v1/reply/lcrenderer_source_checks.json'
                graph=json.loads(path.read_text());graph['dependencies'].pop();path.write_text(json.dumps(graph))
            return result
        with patch.object(runner,'rhi_selection',return_value=self.selected), \
                patch.object(runner,'run_command',side_effect=command):
            with self.assertRaises(ValueError):runner.build_rhi_source(self.args)
        self.assertEqual(1,len(self.commands))

    def test_source_rechecks_aggregate_dependencies_before_publishing_success(self):
        def command(argv,where,timeout,env):
            result=self.command(argv,where,timeout,env)
            if where.name=='cpu-tests':
                path=where.parent/'build/.cmake/api/v1/reply/lcrenderer_source_checks.json'
                graph=json.loads(path.read_text());graph['dependencies'].pop();path.write_text(json.dumps(graph))
            return result
        with patch.object(runner,'rhi_selection',return_value=self.selected), \
                patch.object(runner,'run_command',side_effect=command):
            with self.assertRaises(ValueError):runner.build_rhi_source(self.args)

    def test_source_deadline_rounding_is_capped_without_extending_the_profile(self):
        self.args.timeout=300
        self.assertGreater((212.007+300)-212.007,300)
        def command(argv,where,timeout,env):
            self.assertEqual(300,timeout)
            return self.command(argv,where,timeout,env)
        with patch.object(runner.time,'monotonic',return_value=212.007), \
                patch.object(runner,'rhi_selection',return_value=self.selected), \
                patch.object(runner,'run_command',side_effect=command):
            runner.build_rhi_source(self.args)
        self.args.app=None;self.args.build_dir=None
        with patch.object(runner.time,'monotonic',side_effect=[212.007,512.008]), \
                patch.object(runner,'rhi_selection',return_value=self.selected),patch.object(runner,'run_command') as run:
            with self.assertRaisesRegex(ValueError,'deadline expired'):runner.build_rhi_source(self.args)
            run.assert_not_called()

    def test_source_rejects_reused_builds_or_case_specific_tools(self):
        for key,value in [('app',self.root/'app'),('build_dir',self.root/'build'),('_worker_directory',self.root/'worker'),('case_cmake_option',['ordinary:x=y'])]:
            setattr(self.args,key,value)
            with self.subTest(key=key),self.assertRaises(ValueError):self.execute()
            setattr(self.args,key,[] if key=='case_cmake_option' else None)

    def test_host_or_dual_selection_is_not_silently_replaced_with_gui_less(self):
        for flag in ('WITH_QT_UI','WITH_LUACMDINTERFACE','WITH_PYTHONSCRIPT','WITH_LEGACY_OPENGL_RENDERER','WITH_CAIRO'):
            self.selected['configuration']['expected_options'][flag]=True
            with self.subTest(flag=flag),self.assertRaisesRegex(ValueError,'host/dual'):self.execute()
            self.selected['configuration']['expected_options'][flag]=False

    def test_wrong_prelude_or_failed_cpu_execution_rejected(self):
        self.noise=True
        with self.assertRaises(ValueError):self.execute()
        self.noise=False;self.failure=True
        with self.assertRaises(ValueError):self.execute()

    def test_source_sdk_flags_cannot_override_fixture_options(self):
        self.args.cmake_option=['WITH_QT_UI=ON']
        with self.assertRaisesRegex(ValueError,'fixture-owned'):self.execute()
        self.assertEqual([],self.commands)

    def test_windows_macro_and_compiler_flags_preserve_the_contract_api(self):
        source=(inventory.REPO_ROOT/'unittest/rendering/buildcontracts.cpp').read_text()
        self.assertLess(source.index('#include <windows.h>'),source.index('#undef GetObject'))
        self.assertLess(source.index('#undef GetObject'),source.index('value.GetObject()'))
        cmake=(inventory.REPO_ROOT/'CMakeLists.txt').read_text()
        for name in ('CMAKE_CXX_FLAGS','CMAKE_C_FLAGS'):
            self.assertIn('string(APPEND '+name+' ',cmake)
            self.assertNotIn('SET('+name+' ${'+name+'}',cmake)

    def test_public_header_input_drift_invalidates_the_whole_source_run(self):
        with patch.object(runner,'cpu_header_inputs',side_effect=[{'fixture':'before'},{'fixture':'changed'}]):
            with self.assertRaisesRegex(ValueError,'changed during execution'):self.execute()

    def test_full_cpu_source_map_is_retained_and_source_drift_rejected(self):
        original={'lcviewernoqt/painters/pathstroke.cpp':'before'}
        with patch.object(runner,'production_inputs',return_value=original) as inputs:
            directory=self.execute()
        self.assertEqual(2,inputs.call_count)
        self.assertEqual(original,json.loads((directory/'source-build.json').read_text())['source_sha256'])
        self.args.app=None;self.args.build_dir=None
        with patch.object(runner,'production_inputs',side_effect=[original,{**original,next(iter(original)):'changed'}]):
            with self.assertRaisesRegex(ValueError,'changed during execution'):self.execute()

    def test_linux_worker_removes_only_build_arguments_and_never_rebuilds(self):
        self.args.app=Path('/owned/app');self.args.build_dir=Path('/owned/build')
        argv=['--rhi-contracts','--build-source','--generator','Ninja Multi-Config','--cmake=/sdk/cmake',
              '--library-matrix','--case-cmake-option','dual:CMAKE_TOOLCHAIN_FILE=/sdk/dual',
              '--library-group','python','--library-group=core',
              '--case-cmake-option=cpu:CMAKE_TOOLCHAIN_FILE=/sdk/cpu',
              '--cmake-option','Qt6_DIR=/sdk path','--cmake-option=LUA_LIBRARY=/sdk/lib',
              '--weston','/owned/weston','--timeout','200']
        self.assertEqual(['--rhi-contracts','--weston','/owned/weston','--timeout','200','--app','/owned/app',
                          '--build-dir','/owned/build'],runner.source_worker_argv(argv,self.args))
        with self.assertRaises(ValueError):runner.source_worker_argv(['--cmake-option'],self.args)

    def test_cli_source_mode_is_explicit_not_a_spike_or_cpu_alias(self):
        for flags in (['--build-source'],['--build-source','--spike'],['--build-source','--build-contracts'],['--library-matrix'],
                ['--build-contracts','--library-group','python']):
            result=subprocess.run([sys.executable,'-B',runner.__file__,*flags,'--report-dir',str(self.root)],capture_output=True,timeout=10)
            self.assertEqual(2,result.returncode)


def library_graph(build,flags):
    """Create deliberately synthetic File API/trace/cache data for parser checks."""
    build.mkdir(parents=True,exist_ok=True);reply=build/'.cmake/api/v1/reply';reply.mkdir(parents=True)
    names=['lckernel','lcviewernoqt','lcluascript','lcscripting']
    if flags['WITH_UNITTESTS']:names+=['lcunittest','lcbuildcontracttest','lcviewer_cpu_headercheck']
    if flags['WITH_LEGACY_OPENGL_RENDERER']:names+=['lcviewer_opengl']
    if flags['WITH_RHI_RENDERER']:names+=['lcrenderer_rhi']
    if flags.get('WITH_RENDERER_TESTS'):names+=['lcrenderertest','lcrenderer_headercheck','lcrenderer_source_checks']
    if flags['WITH_PYTHONSCRIPT']:names+=['lcpythonscript','lcuipy_build_tree']
    for name in names:
        sources=[dict(path='lcviewernoqt/painters/opengl/createpainter.cpp' if name=='lcviewer_opengl' else name+'/fixture.cpp')]
        if name=='lcunittest' and flags['WITH_PYTHONSCRIPT']:sources+=[dict(path=p) for p in runner.PYTHON_TEST_SOURCES]
        if name=='lcunittest' and flags['WITH_LEGACY_OPENGL_RENDERER']:sources+=[dict(path='unittest/lcviewernoqt/fontbooktest.cpp')]
        (reply/(name+'.json')).write_text(json.dumps(dict(name=name,id=name+'::fixture',sources=sources,type='SHARED_LIBRARY',artifacts=[dict(path='lib/'+name+'.fixture')])))
        if name in runner.SOURCE_PRIVATE_PCH:
            path=reply/(name+'.json');data=json.loads(path.read_text())
            directory={'lcluascript':'lcadluascript','lcunittest':'unittest'}.get(name,name)
            standard,headers=runner.SOURCE_PRIVATE_PCH[name];standard=standard or runner.project_cxx_standard()
            owner=build/directory/'CMakeFiles'/(name+'.dir');owner.mkdir(parents=True)
            header=owner/'cmake_pch.hxx';source=owner/'cmake_pch.hxx.cxx'
            header.write_text(''.join('#include '+h+'\n' for h in headers));source.write_text('/* synthetic generated PCH source */\n')
            data['paths']=dict(build=directory)
            data['sources'] += [dict(path=str(source),compileGroupIndex=0),dict(path=str(header))]
            data['compileGroups']=[dict(language='CXX',languageStandard=dict(standard=standard),
                precompileHeaders=[dict(header=h) for h in headers]) for _ in range(2)]
            path.write_text(json.dumps(data))
        if name=='lcuipy_build_tree':
            (reply/(name+'.json')).write_text(json.dumps(dict(name=name,id=name+'::fixture',sources=[],type='UTILITY')))
        if name=='lcunittest' or flags.get('WITH_RENDERER_TESTS') and name in ('lcrenderertest','lcrenderer_headercheck'):
            path=reply/(name+'.json');data=json.loads(path.read_text())
            data['type']='OBJECT_LIBRARY' if name=='lcrenderer_headercheck' else 'EXECUTABLE'
            path.write_text(json.dumps(data))
        if name=='lcrenderer_source_checks':
            (reply/(name+'.json')).write_text(json.dumps(dict(name=name,id=name+'::fixture',type='UTILITY',
                dependencies=[dict(id=n+'::fixture') for n in (
                    'lcrenderertest','lcrenderer_headercheck','lcviewer_cpu_headercheck','lcunittest')]))
            )
    directory=build/'lcviewernoqt/cpu-headers';directory.mkdir(parents=True)
    template=(inventory.REPO_ROOT/runner.CPU_HEADER_TEMPLATE).read_text();sources=[];objects=[]
    for header in runner.CPU_PUBLIC_HEADERS:
        path=directory/(re.sub(r'[^A-Za-z0-9_]','_',header)+'.cpp');path.write_text(template.replace('@LC_CPU_HEADER@',header))
        sources.append(dict(path=str(path),compileGroupIndex=0));objects.append(str(path)+'.o')
    (directory/'objects-Release.txt').write_text('\n'.join(objects)+'\n')
    (reply/'lcviewer_cpu_headercheck.json').write_text(json.dumps(dict(name='lcviewer_cpu_headercheck',id='lcviewer_cpu_headercheck::fixture',type='OBJECT_LIBRARY',
        sources=sources,compileGroups=[dict(language='CXX',languageStandard={'standard':'14'})])))
    model=dict(kind='codemodel',version={'major':2},paths=dict(source=str(inventory.REPO_ROOT),build=str(build)),
        configurations=[dict(name='Debug',targets=[]),dict(name='Release',targets=[dict(name=n,id=n+'::fixture',jsonFile=n+'.json') for n in names])])
    (reply/'model.json').write_text(json.dumps(model))
    (reply/'index-fixture.json').write_text(json.dumps(dict(reply={'client-renderer-source':{'codemodel-v2':{'jsonFile':'model.json'}}})))
    (build/'renderer-preflight.json').write_text(json.dumps(dict(evidence_kind='renderer-option-preflight',options=flags)))
    (build/'CMakeCache.txt').write_text(''.join(k+':BOOL='+('ON' if v else 'OFF')+'\n' for k,v in flags.items())+
        ('' if 'LC_RENDERER_SOURCE_PCH' in flags else 'LC_RENDERER_SOURCE_PCH:BOOL=ON\n'))
    packages=['Boost','Eigen3','Lua']+(['GTest'] if flags['WITH_UNITTESTS'] else [])
    if flags['WITH_LEGACY_OPENGL_RENDERER']:packages+=['OpenGL','GLEW','glm','Freetype']
    if flags['WITH_RHI_RENDERER']:packages+=['Qt6','Qt6Gui','Qt6ShaderTools']
    if flags['WITH_PYTHONSCRIPT']:
        packages+=['Python'];sdk=build.parent.parent.parent/'synthetic-sdk';sdk.mkdir(exist_ok=True)
        (sdk/'Python.h').write_text('not-real-Python-headers')
        (build/'python-runtime-inventory.txt').write_text('provider_implementation=CPython\nprovider_version=3.12.10\n'
            'provider_soabi=synthetic-only\nprovider_pointer_bits=64\nexecutable='+sys.executable+'\nlink_library='+sys.executable+'\ninclude_dirs='+str(sdk)+'\n')
    (build.parent/'trace.jsonl').write_text(json.dumps(dict(version={'major':1}))+'\n'+''.join(
        json.dumps(dict(cmd='find_package',args=[p]))+'\n' for p in packages))


def host_graph(build,flags):
    """Synthetic host graph/SDK only; never treated as native acceptance."""
    library_graph(build,flags)
    reply=build/'.cmake/api/v1/reply'
    model=json.loads((reply/'model.json').read_text())
    names={r['name'] for r in model['configurations'][1]['targets']}
    if flags['WITH_PERSISTENCE']:names.update(('persistence','dxfrw'))
    if flags['WITH_LUACMDINTERFACE']:names.add('luacmdinterface')
    if flags['WITH_QT_UI']:
        names.update(('librecad','lcui_objects','lcui_bindings_objects','lcui_widgets_objects'))
        if flags['WITH_UNITTESTS']:names.add('lcui')
    if flags['WITH_PYTHONSCRIPT']:names.add('lcuipy_build_tree')
    model['configurations'][1]['targets']=[dict(name=n,id=n+'::fixture',jsonFile=n+'.json') for n in sorted(names)]
    (reply/'model.json').write_text(json.dumps(model))
    object_paths=['lcUI/CMakeFiles/lcui_objects.dir/'+n+'.o' for n in ('main.cpp','mocs_compilation.cpp','qrc_resource.cpp')]
    bindings=['lcUI/lua/guibridge.cpp','lcUI/managers/copymanager.cpp']+(
        ['lcUI/python/pyguibridge.cpp'] if flags['WITH_PYTHONSCRIPT'] else [])
    binding_paths=['lcUI/CMakeFiles/lcui_bindings_objects.dir/'+Path(n).name+'.o' for n in bindings]
    widgets=['lcUI/widgets/guiAPI/inputgui.cpp']
    widget_paths=['lcUI/CMakeFiles/lcui_widgets_objects.dir/inputgui.cpp.o']
    for name in names:
        path=reply/(name+'.json')
        data=json.loads(path.read_text()) if path.exists() else dict(name=name,sources=[],artifacts=[dict(path='lib/'+name+'.fixture')])
        data.update(id=name+'::fixture',type='SHARED_LIBRARY',dependencies=[])
        if name=='lcviewer_cpu_headercheck':data['type']='OBJECT_LIBRARY'
        if name in ('lckernel','lcviewernoqt'):data.setdefault('compileGroups',[dict(language='CXX',languageStandard=dict(standard=runner.project_cxx_standard()))])
        if name in ('lcui_objects','lcui_bindings_objects','lcui_widgets_objects'):
            sources=([str(build/'lcUI'/n) for n in ('main.cpp','mocs_compilation.cpp','qrc_resource.cpp')]
                if name=='lcui_objects' else widgets if name=='lcui_widgets_objects' else bindings)
            data.update(type='OBJECT_LIBRARY',artifacts=[dict(path=p) for p in (object_paths if name=='lcui_objects' else widget_paths if name=='lcui_widgets_objects' else binding_paths)],
                sources=[dict(path=p,compileGroupIndex=0) for p in sources],
                compileGroups=[dict(language='CXX',languageStandard=dict(standard='17'),
                    defines=[dict(define='LC_WITH_PYTHONSCRIPT')] if flags['WITH_PYTHONSCRIPT'] else [])],
                dependencies=[dict(id='lcuipy_build_tree::fixture')] if flags['WITH_PYTHONSCRIPT'] else [])
            if name=='lcui_widgets_objects':data['dependencies'].append(dict(id='lcui_objects::fixture'))
        if name in ('librecad','luacmdinterface','lcui'):
            data['dependencies']=[dict(id='lcviewer_opengl::fixture')]
            if name!='luacmdinterface':
                data['dependencies'] += [dict(id=n+'::fixture') for n in ('lcui_objects','lcui_bindings_objects','lcui_widgets_objects')]
                data['sources']=[dict(path=str(build/p),isGenerated=True) for p in object_paths+binding_paths+widget_paths]
        if name in ('librecad','luacmdinterface','lcunittest'):
            data.update(type='EXECUTABLE',artifacts=[dict(path='bin/'+name+('.exe' if sys.platform=='win32' else ''))])
        if name=='lcunittest':
            data['sources']+=[dict(path='unittest/'+n) for n in (runner.UI_TEST_SOURCES if flags['WITH_QT_UI'] else ())+(
                ('persistence/dxfexporttest.cpp',) if flags['WITH_PERSISTENCE'] else ())+
                (('lcviewernoqt/testcairofactory.cpp',) if flags['WITH_CAIRO'] else ())+
                (('python/persistence_test.cpp','scripting/luapersistence_test.cpp') if flags['WITH_PYTHONSCRIPT'] and flags['WITH_PERSISTENCE'] else ())]
        if name=='lcuipy_build_tree':data.update(type='UTILITY',artifacts=[])
        path.write_text(json.dumps(data))
    sdk=build.parent/'synthetic-host-sdk';sdk.mkdir()
    with (build/'CMakeCache.txt').open('a') as stream:
        for name in (('Qt6Core','Qt6Gui') if flags['WITH_QT_UI'] or flags['WITH_RHI_RENDERER'] else ())+(
                ('Qt6Widgets','Qt6UiTools','Qt6OpenGL','Qt6OpenGLWidgets','Qt6Svg') if flags['WITH_QT_UI'] else ())+(
                ('Qt6ShaderTools',) if flags['WITH_RHI_RENDERER'] else ()):
            prefix=sdk/name;prefix.mkdir();(prefix/(name+'Config.cmake')).write_text('# Synthetic Qt metadata')
            stream.write(name+'_DIR:PATH='+str(prefix)+'\n')
    with (build.parent/'trace.jsonl').open('a') as stream:
        for name in (('Qt6',) if flags['WITH_QT_UI'] or flags['WITH_RHI_RENDERER'] else ())+(
                ('glfw3','CURL') if flags['WITH_LUACMDINTERFACE'] else ()):
            stream.write(json.dumps(dict(cmd='find_package',args=[name]))+'\n')
        if flags['WITH_CAIRO']:stream.write(json.dumps(dict(cmd='pkg_check_modules',args=['CAIRO','REQUIRED','cairo>=1.13']))+'\n')


class InstalledCliTests(unittest.TestCase):
    def setUp(self):
        temporary=tempfile.TemporaryDirectory(prefix='installed-cli-fixture-');self.addCleanup(temporary.cleanup)
        self.root=Path(temporary.name).resolve();self.build=self.root/'build';self.build.mkdir()
        self.old=self.root/'stage';self.prefix=self.root/'relocated-prefix'
        (self.prefix/'lib').mkdir(parents=True);(self.prefix/'bin').mkdir()
        self.files={'lckernel':'lib/liblckernel.so','lcviewer_opengl':'lib/liblcviewer_opengl.so','luacmdinterface':'bin/luacmdinterface'}
        self.targets={n:dict(artifacts=[dict(path='build/'+Path(p).name)],install=dict(destinations=[dict(path=str(Path(p).parent))]))
                      for n,p in self.files.items()}
        for p in self.files.values():(self.prefix/p).write_bytes(b'synthetic-not-native')
        (self.build/'install_manifest.txt').write_text(''.join(str(self.old/p)+'\n' for p in self.files.values()))
        patched=patch.object(runner,'source_targets',return_value=self.targets);patched.start();self.addCleanup(patched.stop)

    def inspect(self):return runner.installed_cli_files(self.build,self.prefix,self.old,list(self.files))

    def test_relocated_tree_manifest_and_target_paths_are_independently_bound(self):
        self.targets['lckernel']['install']['destinations']*=2
        record=self.inspect();self.assertEqual(self.files,record['targets']);self.assertEqual(3,len(record['files_sha256']))
        self.assertFalse(self.old.exists())
        for macos in (True,False):
            lines=[('dyld[123]: <ABCD-1234> ' if macos else '123: calling init: ')+str(self.prefix/p)
                   for n,p in self.files.items() if n!='luacmdinterface']
            result=runner.installed_cli_loads('\n'.join(lines).encode(),self.prefix,self.files,macos)
            self.assertEqual(2,len(result['libraries']))
            normalized=[line.replace(str(self.prefix/'lib'),str(self.prefix/'bin/../lib')) for line in lines]
            self.assertEqual(result['libraries'],runner.installed_cli_loads('\n'.join(normalized).encode(),self.prefix,self.files,macos)['libraries'])

    def test_missing_extra_escaping_duplicate_and_foreign_install_files_fail(self):
        manifest=self.build/'install_manifest.txt';original=manifest.read_text();file=self.prefix/self.files['lckernel']
        for change in ('missing','extra','escape','duplicate','foreign','relative'):
            with self.subTest(change=change):
                if change=='missing':file.rename(self.root/'saved')
                if change=='extra':(self.prefix/'extra').write_bytes(b'extra')
                if change=='escape':file.rename(self.root/'saved');file.symlink_to(self.root/'saved')
                if change=='duplicate':manifest.write_text(original+original.splitlines()[0]+'\n')
                if change=='foreign':manifest.write_text('/foreign/liblckernel.so\n')
                if change=='relative':manifest.write_text('lib/liblckernel.so\n')
                with self.assertRaises(ValueError):self.inspect()
                manifest.write_text(original)
                if change in ('missing','escape'):
                    if file.is_symlink():file.unlink()
                    (self.root/'saved').rename(file)
                if change=='extra':(self.prefix/'extra').unlink()

    def test_target_install_cannot_be_absent_ambiguous_external_or_duplicated(self):
        for destinations in ([],[dict(path='lib'),dict(path='other')],[dict(path='/foreign')],[dict(path='../foreign')]):
            with self.subTest(destinations=destinations):
                self.targets['lckernel']['install']['destinations']=destinations
                with self.assertRaises(ValueError):self.inspect()
        self.targets['lckernel']['install']['destinations']=[dict(path='lib')]
        self.targets['lcviewer_opengl']=copy.deepcopy(self.targets['lckernel'])
        with self.assertRaisesRegex(ValueError,'duplicate installed'):self.inspect()

    def test_loader_proof_rejects_missing_build_tree_and_foreign_libraries(self):
        for macos in (True,False):
            head='dyld[123]: ' if macos else '123: calling init: '
            for text in ('',head+str(self.prefix/'lib/liblckernel.so'),
                         head+str(self.root/'build/liblckernel.so')+'\n'+head+str(self.prefix/'lib/liblcviewer_opengl.so')):
                with self.subTest(macos=macos,text=text),self.assertRaises(ValueError):
                    runner.installed_cli_loads(text.encode(),self.prefix,self.files,macos)

    def test_cmake_installs_cli_and_gives_both_hosts_relative_loader_paths(self):
        root=(inventory.REPO_ROOT/'CMakeLists.txt').read_text()
        cli=(inventory.REPO_ROOT/'luacmdinterface/CMakeLists.txt').read_text()
        self.assertIn('install(TARGETS luacmdinterface\n    ${LC_INSTALL_RUNTIME_DEPENDENCY_SET_ARGS}',cli)
        block=root.split('# library-to-library dependencies from each library\'s own directory.\n',1)[1].split('\n# On macOS, Python\'s embed target',1)[0]
        fixture=self.root/'rpath-fixture';fixture.mkdir()
        (fixture/'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.28)
project(RpathFixture NONE)
set(APPLE "${FIXTURE_APPLE}")
set(UNIX "${FIXTURE_UNIX}")
set(CMAKE_INSTALL_LIBDIR chosen-lib)
foreach(name IN LISTS FIXTURE_TARGETS)
    add_library(${name} INTERFACE)
    set_target_properties(${name} PROPERTIES INSTALL_RPATH untouched)
endforeach()
'''+block+'''
file(WRITE "${CMAKE_BINARY_DIR}/rpaths.txt" "")
foreach(name IN LISTS FIXTURE_TARGETS)
    get_target_property(value ${name} INSTALL_RPATH)
    file(APPEND "${CMAKE_BINARY_DIR}/rpaths.txt" "${name}=${value}\\n")
endforeach()
''')
        shared=('lckernel','lcscripting','persistence','lcluascript','lcpythonscript','lcviewernoqt','lcviewer_opengl','lcui')
        hosts=('librecad','luacmdinterface')
        for platform,apple,unix,library,host in (
                ('macos','ON','ON','@loader_path','@executable_path/../chosen-lib'),
                ('linux','OFF','ON','$ORIGIN','$ORIGIN/../chosen-lib'),
                ('windows','OFF','OFF','untouched','untouched')):
            for index,selected in enumerate((shared+hosts,('lckernel','luacmdinterface'),())):
                with self.subTest(platform=platform,targets=selected):
                    names=(*selected,'unowned');build=fixture/(platform+str(index))
                    result=subprocess.run(['cmake','-S',str(fixture),'-B',str(build),'-DFIXTURE_APPLE='+apple,
                        '-DFIXTURE_UNIX='+unix,'-DFIXTURE_TARGETS='+';'.join(names)],capture_output=True,text=True,timeout=30)
                    self.assertEqual(0,result.returncode,result.stdout+result.stderr)
                    actual=dict(line.split('=',1) for line in (build/'rpaths.txt').read_text().splitlines())
                    self.assertEqual({name:library if name in shared else host if name in hosts else 'untouched'
                                      for name in names},actual)

    def test_windows_runtime_artifacts_and_actual_module_report_are_bound(self):
        (self.build/'CMakeCache.txt').write_text('CMAKE_INSTALL_BINDIR:PATH=bin\n')
        for folder in ('lib','bin'):
            for path in (self.prefix/folder).iterdir():path.unlink()
        files={'lckernel':'bin/lckernel.dll','luacmdinterface':'bin/luacmdinterface.exe'}
        self.targets.clear()
        for name,value in files.items():
            (self.prefix/value).write_bytes(b'synthetic PE runtime, not native evidence')
            self.targets[name]=dict(install=dict(destinations=[dict(path='lib'),dict(path='bin')]),
                artifacts=[dict(path=value)]+([dict(path='lib/lckernel.lib')] if name=='lckernel' else []))
        manifest=self.build/'install_manifest.txt'
        manifest.write_text(''.join(str(self.old/value)+'\n' for value in files.values())+str(self.old/files['lckernel'])+'\n')
        result=runner.installed_cli_files(self.build,self.prefix,self.old,list(files),windows=True)
        self.assertEqual(files,result['targets'])
        (self.build/'CMakeCache.txt').unlink() # Conan may set the effective bindir without any cache entry.
        self.assertEqual(result,runner.installed_cli_files(self.build,self.prefix,self.old,list(files),windows=True))
        extra=self.prefix/'lib/lckernel.dll';extra.write_bytes(b'ambiguous second runtime')
        manifest.write_text(manifest.read_text()+str(self.old/'lib/lckernel.dll')+'\n')
        with self.assertRaisesRegex(ValueError,'ambiguous CLI target'):
            runner.installed_cli_files(self.build,self.prefix,self.old,list(files),windows=True)
        extra.unlink();manifest.write_text(''.join(str(self.old/value)+'\n' for value in files.values())+str(self.old/files['lckernel'])+'\n')
        report='\n'.join(str(self.prefix/value) for value in files.values()).encode()
        observed=runner.installed_cli_loads(report,self.prefix,files,False,windows=True)
        self.assertEqual({'lckernel.dll':str(self.prefix/files['lckernel'])},observed['libraries'])
        with self.assertRaises(ValueError):runner.installed_cli_loads(b'',self.prefix,files,False,windows=True)
        source=(inventory.REPO_ROOT/'luacmdinterface/moduleinventory.h').read_text()
        self.assertIn('CreateToolhelp32Snapshot',source);self.assertIn('Module32FirstW',source)
        main=(inventory.REPO_ROOT/'luacmdinterface/main.cpp').read_text()
        self.assertLess(main.index('writeRendererModuleInventory()'),main.index('// Create Librecad document'))

    @unittest.skipUnless(shutil.which('cmake'),'CMake syntax check requires CMake')
    def test_runtime_dependency_search_arguments_parse_before_artifact_options(self):
        source=(inventory.REPO_ROOT/'CMakeLists.txt').read_text()
        block=source[source.index('    install(RUNTIME_DEPENDENCY_SET LibreCADRuntimeDependencies'):]
        block=block[:block.index('\nendif()')]
        (self.root/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.21)\nproject(Syntax NONE)\n'
            'set(CMAKE_INSTALL_BINDIR bin)\nset(_lc_runtime_dependency_directory_args DIRECTORIES /sdk/bin)\n'+block+'\n')
        result=subprocess.run(['cmake','-S',str(self.root),'-B',str(self.root/'syntax')],capture_output=True,text=True,timeout=30)
        self.assertEqual(0,result.returncode,result.stdout+result.stderr)


class HostSourceTests(unittest.TestCase):
    def setUp(self):
        SourceBuildTests.setUp(self)
        native=patch.object(runner,'native_platform',return_value='LINUX-ARM64');native.start();self.addCleanup(native.stop)
        self.args.platform='LINUX-ARM64';self.args.host_profile='legacy';self.args.host_tests='ON';self.args.host_cairo=True
        self.args.configuration=None
        self.args.host_kind='both';self.args.host_no_persistence=False;self.args.host_install=False

    def command(self,command,where,timeout,env,expected_exit=0):
        self.commands.append((command,env))
        self.assertLessEqual(timeout,self.args.timeout)
        if where.name=='configure':
            flags={v[2:].split(':BOOL=')[0]:v.endswith('=ON') for v in command if ':BOOL=' in v}
            self.host_flags=flags
            host_graph(Path(command[command.index('-B')+1]),flags)
        if where.name.startswith('library-') or where.name in ('ui-bindings','ui-objects','ui-widgets','hosts','tests','install-prerequisites'):
            build=Path(command[2])
            for target in runner.source_targets(build).values():
                for a in target.get('artifacts',[]):
                    path=build/a['path'];path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(b'synthetic-only')
            for name in (build/'lcviewernoqt/cpu-headers/objects-Release.txt').read_text().splitlines():Path(name).write_bytes(b'synthetic-only')
            if where.name=='hosts' and getattr(self,'change_sdk',False):
                (build.parent/'synthetic-host-sdk/Qt6Core/Qt6CoreConfig.cmake').write_bytes(b'changed synthetic SDK')
        if where.name=='cli-help':
            self.assertEqual(1,expected_exit);(where/'stdout.log').write_bytes(b'Allowed options\n')
        if where.name=='discovery':
            ui=self.host_flags['WITH_QT_UI']
            (where/'stdout.log').write_bytes(b'LibreCAD v3 (synthetic)\nS.\n  A\n  B\n'+(b'UI.\n  Unexecuted\n' if ui else b''))
            suites=[dict(name='S',tests=2,testsuite=[dict(name=n,file=str(inventory.REPO_ROOT/'unittest/lckernel/math/code.h'),line=1) for n in ('A','B')])]
            if ui:suites.append(dict(name='UI',tests=1,testsuite=[dict(name='Unexecuted',file=str(inventory.REPO_ROOT/'unittest/ui/testtoolbar.cpp'),line=1)]))
            (where/'compiled-tests.json').write_text(json.dumps(dict(tests=3 if ui else 2,testsuites=suites)))
        if where.name=='non-widget-tests':
            (where/'tests.xml').write_text('<testsuites tests="2" failures="0" disabled="0" errors="0"><testsuite name="S" tests="2" failures="0" disabled="0" errors="0" skipped="0">'+''.join(
                '<testcase name="'+n+'" classname="S" status="run" result="completed"/>' for n in ('A','B'))+'</testsuite></testsuites>')
        if where.name=='install':
            build=Path(command[2]);prefix=Path(command[command.index('--prefix')+1]);files=[]
            for name,target in runner.source_targets(build).items():
                if name not in ('lckernel','lcviewernoqt','lcviewer_opengl','lcscripting','lcluascript','lcpythonscript','persistence','luacmdinterface'):continue
                destination='bin' if name=='luacmdinterface' else 'lib'
                target['install']=dict(destinations=[dict(path=destination)])
                path=build/'.cmake/api/v1/reply'/(name+'.json');path.write_text(json.dumps(target))
                installed=prefix/destination/Path(target['artifacts'][0]['path']).name
                installed.parent.mkdir(parents=True,exist_ok=True);installed.write_bytes(b'installed synthetic-only')
                files.append(str(installed))
            (build/'install_manifest.txt').write_text('\n'.join(files)+'\n')
        if where.name=='installed-cli-help':
            self.assertEqual(1,expected_exit)
            for key in env:
                self.assertFalse(key.startswith(('DYLD_','LD_')) and key not in ('LD_DEBUG','DYLD_PRINT_LIBRARIES'))
            prefix=Path(command[0]).parent.parent
            self.assertFalse((prefix.parent/'stage').exists())
            (where/'stdout.log').write_bytes(b'Allowed options\n')
            (where/'stderr.log').write_text('\n'.join(('dyld[42]: <ABCD-1234> ' if sys.platform=='darwin' else '42: calling init: ')+str(p)
                for p in sorted((prefix/'lib').iterdir())))
        return dict(synthetic_only=True)

    def execute(self):
        with patch.object(runner,'run_command',side_effect=self.command):
            return runner.build_host_source(self.args)

    def test_all_eight_host_modes_build_actual_hosts_and_keep_test_selection(self):
        for profile in runner.HOST_PROFILES:
            for tests in ('ON','OFF'):
                with self.subTest(profile=profile,tests=tests):
                    self.args.host_profile=profile;self.args.host_tests=tests;self.commands=[]
                    with patch.dict(os.environ,GTEST_FILTER='Wrong.Suite',DISPLAY=':999',PYTHONPATH='/poison',SOURCE_DATE_EPOCH='poison'), \
                            patch.object(runner.time,'time',return_value=1700000000.9):
                        directory=self.execute()
                    record=json.loads((directory/'host-build.json').read_text())
                    self.assertFalse(record['phase_or_platform_accepted']);self.assertFalse(record['runtime_graphics_executed'])
                    self.assertEqual(['S.A','S.B'] if tests=='ON' else [],record['executed_non_widget_ids'])
                    self.assertEqual(['UI.Unexecuted'] if tests=='ON' else [],record['unexecuted_widget_ids'])
                    for name in ('lcui_objects','lcui_bindings_objects','lcui_widgets_objects'):self.assertIn(name,record['artifacts_sha256'])
                    self.assertEqual(1200,record['profile_budget_seconds'])
                    stages=[c[c.index('--target')+1] for c,e in self.commands if '--target' in c]
                    self.assertLess(stages.index('lcui_bindings_objects'),stages.index('lcui_objects'))
                    self.assertLess(stages.index('lcui_objects'),stages.index('lcui_widgets_objects'))
                    self.assertLess(stages.index('lcui_widgets_objects'),stages.index('librecad'))
                    configure=self.commands[0][0]
                    for flag in ('WITH_QT_UI','WITH_LUACMDINTERFACE','WITH_PERSISTENCE'):
                        self.assertIn('-D'+flag+':BOOL=ON',configure)
                    self.assertIn('-DLC_RENDERER_SOURCE_PCH:BOOL=ON',configure)
                    self.assertEqual(set(runner.SOURCE_PRIVATE_PCH)-({'lcunittest'} if tests=='OFF' else set()),
                                     set(record['observed']['private_pch']))
                    self.assertEqual(tests=='OFF','-DCMAKE_DISABLE_FIND_PACKAGE_GTest=ON' in configure)
                    for command,env in self.commands:
                        for name in ('GTEST_FILTER','DISPLAY','PYTHONPATH'):self.assertNotIn(name,env)
                        self.assertEqual('1700000000',env['SOURCE_DATE_EPOCH'])
                    self.assertEqual('offscreen',self.commands[-1][1]['QT_QPA_PLATFORM'])
                    libraries=[c for c,e in self.commands if '--target' in c and c[c.index('--target')+1]=='lcluascript']
                    self.assertEqual(1,len(libraries))
                    self.assertEqual(['lcluascript','--parallel',runner.source_parallelism()],libraries[0][libraries[0].index('--target')+1:])

    def test_cli_installs_move_before_loading_and_preserve_owned_artifacts(self):
        self.args.host_kind='cli';self.args.host_no_persistence=True;self.args.host_install=True
        for python in (False,True):
            for tests in ('ON','OFF'):
                self.commands=[]
                self.args.host_profile='legacy-python' if python else 'legacy';self.args.host_tests=tests
                with self.subTest(python=python,tests=tests),patch.dict(os.environ,LD_LIBRARY_PATH='/poison',
                        DYLD_LIBRARY_PATH='/poison',DYLD_INSERT_LIBRARIES='/poison',DESTDIR='/foreign'):
                    directory=self.execute()
                record=json.loads((directory/'host-build.json').read_text());installed=record['installation']
                self.assertFalse(record['installed_python_executed']);self.assertFalse(record['runtime_graphics_executed'])
                self.assertEqual(6 if python else 5,len(installed['loads']['libraries']))
                self.assertEqual(set(installed['targets'])-{'luacmdinterface'},set(record['artifacts_sha256'])-{'luacmdinterface','lcunittest'})
                self.assertEqual(installed['files_sha256'],runner.installed_cli_files(directory/'build',directory/'relocated-prefix',
                    directory/'stage',list(installed['targets']))['files_sha256'])
                self.assertNotIn('DESTDIR',self.commands[-1][1])
                stages=[c[0] for c in self.commands]
                prerequisites=next(c for c in stages if '--target' in c and c[c.index('--target')+1]=='tinyspline')
                self.assertLess(stages.index(prerequisites),next(i for i,c in enumerate(stages) if '--help' in c))

    def test_install_request_cannot_silently_drop_ui_or_accept_foreign_host(self):
        self.args.host_install=True
        for kind in ('both','ui'):
            self.args.host_kind=kind
            with self.subTest(kind=kind),self.assertRaisesRegex(ValueError,'CLI install'):self.execute()
        self.assertEqual([],self.commands)

    def test_host_rejects_foreign_platform_reuse_and_fixture_sdk_overrides(self):
        for key,value in (('platform','FOREIGN'),('build_dir',self.root/'old'),('configuration','graphics'),
                          ('case_cmake_option',['ordinary:CMAKE_TOOLCHAIN_FILE=/foreign']),('cmake_option',['WITH_QT_UI=OFF'])):
            original=getattr(self.args,key,None);setattr(self.args,key,value)
            with self.subTest(key=key),self.assertRaises(ValueError):self.execute()
            setattr(self.args,key,original)
        self.assertEqual([],self.commands)

    def test_real_object_graph_rejects_duplicate_missing_leaked_or_recompiled_units(self):
        directory=self.execute();build=directory/'build';reply=build/'.cmake/api/v1/reply'
        objects=reply/'lcui_objects.json';consumer=reply/'lcui.json';cpu=reply/'lcviewernoqt.json'
        flags=json.loads((directory/'host-build.json').read_text())['options']
        for path,change in ((objects,lambda d:d['artifacts'].pop()),
                (objects,lambda d:d['compileGroups'][0]['languageStandard'].update(standard='14')),
                (objects,lambda d:d['compileGroups'][0]['defines'].append(dict(define='LC_WITH_PYTHONSCRIPT'))),
                (consumer,lambda d:d['sources'].pop()),
                (consumer,lambda d:d['sources'].append(dict(path='lcUI/main.cpp',compileGroupIndex=0))),
                (cpu,lambda d:d['compileGroups'][0]['languageStandard'].update(standard='14'))):
            original=path.read_text();data=json.loads(original);change(data);path.write_text(json.dumps(data))
            with self.subTest(path=path),self.assertRaises(ValueError):runner.host_object_graph(build,flags)
            path.write_text(original)

    def test_bindings_group_cannot_be_missing_duplicated_or_regenerated(self):
        directory=self.execute();build=directory/'build';reply=build/'.cmake/api/v1/reply'
        flags=json.loads((directory/'host-build.json').read_text())['options']
        bindings=reply/'lcui_bindings_objects.json';consumer=reply/'librecad.json'
        for path,change in ((bindings,lambda d:d['sources'].pop()),
                (bindings,lambda d:d['sources'].append(d['sources'][0])),
                (bindings,lambda d:d['compileGroups'][0]['languageStandard'].update(standard='14')),
                (bindings,lambda d:d['sources'][0].update(path='lcUI/main.cpp')),
                (consumer,lambda d:d['dependencies'].pop())):
            original=path.read_text();data=json.loads(original);change(data);path.write_text(json.dumps(data))
            with self.subTest(path=path),self.assertRaises(ValueError):runner.host_object_graph(build,flags)
            path.write_text(original)

    def test_widgets_require_owned_plain_sources_and_single_generator_dependency(self):
        directory=self.execute();build=directory/'build';reply=build/'.cmake/api/v1/reply'
        flags=json.loads((directory/'host-build.json').read_text())['options']
        path=reply/'lcui_widgets_objects.json';original=path.read_text()
        for change in (lambda d:d['dependencies'].clear(),
                       lambda d:d['sources'][0].update(path='lcUI/main.cpp'),
                       lambda d:d['sources'][0].update(path='lcUI/widgets/scriptdock.h'),
                       lambda d:d['sources'].append(d['sources'][0])):
            data=json.loads(original);change(data);path.write_text(json.dumps(data))
            with self.assertRaises(ValueError):runner.host_object_graph(build,flags)
        path.write_text(original)

    def test_in_checkout_generated_object_sources_use_source_not_build_base(self):
        directory=self.execute();build=directory/'build';reply=build/'.cmake/api/v1/reply'
        flags=json.loads((directory/'host-build.json').read_text())['options']
        expected=runner.host_object_graph(build,flags)
        for name in ('librecad','lcui'):
            path=reply/(name+'.json');data=json.loads(path.read_text())
            for item in data['sources']:
                item['path']=str(Path(item['path']).relative_to(self.root.resolve()))
            path.write_text(json.dumps(data))
        model=reply/'model.json';data=json.loads(model.read_text())
        data['paths']['source']=str(self.root.resolve());model.write_text(json.dumps(data))
        with patch.object(inventory,'REPO_ROOT',self.root.resolve()):
            self.assertEqual(expected,runner.host_object_graph(build,flags))

    def test_ci_generated_source_tree_is_ignored_but_renderer_inputs_are_not(self):
        result=subprocess.run(['git','check-ignore','--no-index','--stdin'],cwd=inventory.REPO_ROOT,
            input='build-r1/hosts/build/lckernel/build_constants.cpp\nlcrenderer/rhi/src/factory.cpp\n',
            capture_output=True,text=True,timeout=10)
        self.assertEqual(0,result.returncode,result.stderr)
        self.assertEqual(['build-r1/hosts/build/lckernel/build_constants.cpp'],result.stdout.splitlines())

    def test_visual_studio_ui_objects_bind_release_configuration_exactly(self):
        directory=self.execute();build=directory/'build';reply=build/'.cmake/api/v1/reply'
        flags=json.loads((directory/'host-build.json').read_text())['options']
        for name in ('lcui_objects','lcui_bindings_objects','lcui_widgets_objects'):
            objects=reply/(name+'.json');data=json.loads(objects.read_text())
            for source in data['sources']:source['path']=source['path'].replace('mocs_compilation.cpp','mocs_compilation_Release.cpp')
            for artifact in data['artifacts']:artifact['path']=artifact['path'].replace('.dir/','.dir/Release/')
            objects.write_text(json.dumps(data))
        for name in ('librecad','lcui'):
            path=reply/(name+'.json');data=json.loads(path.read_text())
            for source in data['sources']:source['path']=source['path'].replace('.dir/','.dir/$(Configuration)/')
            path.write_text(json.dumps(data))
        self.assertEqual(6,runner.host_object_graph(build,flags)['ui_compilations'])
        path=reply/'lcui.json';path.write_text(path.read_text().replace('$(Configuration)','Debug'))
        with self.assertRaisesRegex(ValueError,'shared UI objects'):runner.host_object_graph(build,flags)

    def test_compiled_selection_handles_header_parameterized_and_conditional_tests(self):
        source=(self.root/'fixture-source').resolve();folder=source/'unittest';folder.mkdir(parents=True)
        (folder/'code.h').write_text('TEST(Headers, Kept) {}\n');(folder/'code.cpp').write_text('#if 0\nTEST(Core, NotCompiled) {}\n#endif\nTEST_P(Core, Kept) {}\n')
        (folder/'ui').mkdir();(folder/'ui/widget.cpp').write_text('TEST(UI, NeverRun) {}\n')
        inputs={str(p.relative_to(source)):runner.sha256(p.read_bytes()) for p in folder.rglob('*') if p.is_file()}
        data=dict(tests=3,testsuites=[dict(name=s,tests=1,testsuite=[dict(name=n,file=str(folder/p),line=line)])
            for s,n,p,line in [('Headers','Kept','code.h',1),('Core/0','Kept/0','code.cpp',4),('UI','NeverRun','ui/widget.cpp',1)]])
        ids=['Headers.Kept','Core/0.Kept/0','UI.NeverRun']
        with patch.object(inventory,'REPO_ROOT',source):
            actual=runner.host_test_selection(json.dumps(data).encode(),ids,inputs)
            self.assertEqual(['Core/0.Kept/0','Headers.Kept'],actual['required_ids'])
            self.assertEqual(['UI.NeverRun'],actual['widget_ids'])
            with self.assertRaisesRegex(ValueError,'differs'):
                runner.host_test_selection(json.dumps(data).encode(),ids+['Core.NotCompiled'],inputs)
            (folder/'code.h').write_text('// changed header\n')
            with self.assertRaisesRegex(ValueError,'changed compiled'):
                runner.host_test_selection(json.dumps(data).encode(),ids,inputs)

    def test_independent_host_kinds_and_explicit_persistence_off_are_not_fallbacks(self):
        lua=(inventory.REPO_ROOT/'lcadluascript/CMakeLists.txt').read_text()
        self.assertRegex(lua,r'if\(WITH_PERSISTENCE\)\s+target_link_libraries\(lcluascript persistence\)\s+endif\(\)')
        for kind,no_persistence in (('ui',False),('cli',False),('cli',True)):
            for profile in runner.HOST_PROFILES:
                for tests in ('ON','OFF'):
                    with self.subTest(kind=kind,persistence=not no_persistence,profile=profile,tests=tests):
                        self.args.host_kind=kind;self.args.host_no_persistence=no_persistence
                        self.args.host_profile=profile;self.args.host_tests=tests;self.args.host_cairo=False
                        self.commands=[]
                        directory=self.execute();record=json.loads((directory/'host-build.json').read_text())
                        self.assertEqual(kind,record['host_kind']);self.assertEqual(kind=='cli',record['cli_help_executed'])
                        names=record['artifacts_sha256']
                        self.assertEqual(kind=='ui','librecad' in names);self.assertEqual(kind=='cli','luacmdinterface' in names)
                        self.assertEqual(not no_persistence,'persistence' in names)
                        stages=[c[c.index('--target')+1] for c,e in self.commands if '--target' in c]
                        if not no_persistence:self.assertLess(stages.index('persistence'),stages.index('lcluascript'))
                        else:self.assertNotIn('persistence',stages)
                        self.assertEqual((7 if 'python' in profile else 6) if kind=='ui' else 0,record['ui_objects']['ui_compilations'])
                        self.assertEqual(['UI.Unexecuted'] if kind=='ui' and tests=='ON' else [],record['unexecuted_widget_ids'])
        self.args.host_kind='ui';self.args.host_no_persistence=True
        with self.assertRaisesRegex(ValueError,'Persistence OFF'):self.execute()

    def test_lua_persistence_test_tracks_both_feature_flags(self):
        for no_persistence in (False,True):
            self.args.host_kind='cli';self.args.host_no_persistence=no_persistence
            self.args.host_profile='legacy-python';self.args.host_cairo=False
            directory=self.execute();build=directory/'build'
            flags=json.loads((directory/'host-build.json').read_text())['options']
            path=build/'.cmake/api/v1/reply/lcunittest.json';data=json.loads(path.read_text())
            name='unittest/scripting/luapersistence_test.cpp'
            if no_persistence:data['sources'].append(dict(path=name))
            else:data['sources']=[s for s in data['sources'] if s['path']!=name]
            path.write_text(json.dumps(data))
            with self.subTest(persistence=not no_persistence),self.assertRaisesRegex(ValueError,'Lua persistence test-source membership'):
                runner.library_profile_graph(build,flags,hosts=True)

    def test_missing_generation_dependency_and_empty_cpu_groups_are_rejected(self):
        self.args.host_profile='legacy-python';directory=self.execute();build=directory/'build'
        reply=build/'.cmake/api/v1/reply';flags=json.loads((directory/'host-build.json').read_text())['options']
        for name,key in (('lcui_objects','dependencies'),('lcui_bindings_objects','dependencies'),('lcui_widgets_objects','dependencies'),('lcviewernoqt','compileGroups')):
            path=reply/(name+'.json');original=path.read_text();data=json.loads(original);data[key]=[];path.write_text(json.dumps(data))
            with self.subTest(name=name),self.assertRaises(ValueError):runner.host_object_graph(build,flags)
            path.write_text(original)

    def test_compiled_origin_metadata_cannot_hide_or_misclassify_tests(self):
        origin=str(inventory.REPO_ROOT/'unittest/lckernel/math/code.h')
        data=dict(tests=1,testsuites=[dict(name='S',tests=1,testsuite=[dict(name='A',file=origin,line=1)])])
        inputs=runner.production_inputs(hosts=True)
        for change in (lambda d:d.update(tests=True),lambda d:d['testsuites'][0].update(tests=2),
                lambda d:d['testsuites'][0]['testsuite'][0].pop('file'),
                lambda d:d['testsuites'][0]['testsuite'][0].update(file='/foreign/test.cpp'),
                lambda d:d['testsuites'][0]['testsuite'][0].update(line=0),
                lambda d:d['testsuites'][0]['testsuite'][0].update(line=1000000)):
            value=copy.deepcopy(data);change(value)
            with self.subTest(value=value),self.assertRaises(ValueError):
                runner.host_test_selection(json.dumps(value).encode(),['S.A'],inputs)
        with self.assertRaisesRegex(ValueError,'differs'):
            runner.host_test_selection(json.dumps(data).encode(),['S.A','S.Unaccounted'],inputs)
        inputs.pop('unittest/lckernel/math/code.h')
        with self.assertRaisesRegex(ValueError,'Unbound'):
            runner.host_test_selection(json.dumps(data).encode(),['S.A'],inputs)

    def test_missing_host_test_or_sdk_is_not_replaced_with_gui_less_success(self):
        directory=self.execute();build=directory/'build';reply=build/'.cmake/api/v1/reply'
        flags=json.loads((directory/'host-build.json').read_text())['options']
        target=reply/'lcunittest.json';original=target.read_text();data=json.loads(original)
        data['sources']=[s for s in data['sources'] if not s['path'].endswith('ui/gui/testmenu.cpp')]
        target.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError,'test-source membership'):runner.library_profile_graph(build,flags,hosts=True)
        target.write_text(original)
        # libdxfrw must be the vendored in-tree target, never an external SDK.
        cache=(build/'CMakeCache.txt').read_text()
        (build/'CMakeCache.txt').write_text(cache+'LIBDXFRW_LIBRARY:FILEPATH=/external/libdxfrw.so\n')
        with self.assertRaisesRegex(ValueError,'External libdxfrw SDK'):runner.library_profile_graph(build,flags,hosts=True)
        (build/'CMakeCache.txt').write_text(cache)
        model=reply/'model.json';original=model.read_text();data=json.loads(original)
        data['configurations'][1]['targets']=[t for t in data['configurations'][1]['targets'] if t['name']!='dxfrw']
        model.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError,'target mismatch: dxfrw'):runner.library_profile_graph(build,flags,hosts=True)
        model.write_text(original)

    def test_changed_provider_bytes_invalidate_the_host_result(self):
        self.change_sdk=True
        with self.assertRaisesRegex(ValueError,'changed during execution: graph'):self.execute()
        directory=next(self.root.glob('host-*'));sdk=(directory/'synthetic-host-sdk/Qt6Core/Qt6CoreConfig.cmake').resolve()
        changes=json.loads((directory/'host-state-changes.json').read_text())
        self.assertEqual({'graph'},set(changes));self.assertFalse((directory/'host-build.json').exists())
        before=changes['graph']['before'];after=changes['graph']['after']
        self.assertEqual(runner.sha256(b'# Synthetic Qt metadata'),before['host_sdk_files_sha256'][str(sdk)])
        expected=copy.deepcopy(before);expected['host_sdk_files_sha256'][str(sdk)]=runner.sha256(b'changed synthetic SDK')
        self.assertEqual(expected,after)

    def test_host_end_state_reports_every_changed_component_without_success_receipt(self):
        readers=dict(helpers='helper_digests',inputs='production_inputs',graph='library_profile_graph',
            objects='host_object_graph',artifacts='library_artifacts')
        for changed in ((),*((name,) for name in readers),tuple(readers)):
            with self.subTest(changed=changed):
                reads={name:[] for name in readers};previous=set(self.root.glob('host-*'));self.commands=[]
                def readback(name,reader):
                    def read(*args,**kwargs):
                        value=copy.deepcopy(reader(*args,**kwargs))
                        if reads[name] and name in changed:value['synthetic_changed_end_readback']=True
                        reads[name].append(copy.deepcopy(value));return value
                    return read
                with ExitStack() as stack:
                    for name,attribute in readers.items():
                        stack.enter_context(patch.object(runner,attribute,side_effect=readback(name,getattr(runner,attribute))))
                    if changed:
                        with self.assertRaises(ValueError) as error:self.execute()
                        self.assertEqual('Host inputs, graph or compiled artifacts changed during execution: '+', '.join(changed),
                            str(error.exception))
                    else:self.execute()
                directory,=set(self.root.glob('host-*'))-previous
                self.assertTrue(all(len(values)==2 for values in reads.values()))
                self.assertEqual(not changed,(directory/'host-build.json').exists())
                self.assertEqual(bool(changed),(directory/'host-state-changes.json').exists())
                if changed:
                    self.assertEqual({name:dict(before=reads[name][0],after=reads[name][1]) for name in changed},
                        json.loads((directory/'host-state-changes.json').read_text()))

    def test_cli_host_scope_cannot_alias_graphics_or_silently_disable_a_host(self):
        for options in (['--host-builds'],['--host-profile','legacy','--production-builds','--platform',self.args.platform],
                ['--host-builds','--host-profile','legacy','--platform',self.args.platform,'--backend','vulkan'],
                ['--host-builds','--host-profile','legacy','--platform',self.args.platform,'--host-kind','ui','--host-no-persistence'],
                ['--production-builds','--platform',self.args.platform,'--host-kind','cli']):
            result=subprocess.run([sys.executable,'-B',runner.__file__,*options,'--report-dir',str(self.root)],capture_output=True,timeout=10)
            self.assertEqual(2,result.returncode)


class LibraryMatrixTests(unittest.TestCase):
    def setUp(self):
        SourceBuildTests.setUp(self)
        self.args.library_matrix=True
        self.flags=dict(self.selected['configuration']['expected_options'],WITH_RENDERER_TESTS=False,
            WITH_PERSISTENCE=False,WITH_LIBOPENCAD=False,WITH_DOCUMENTATION=False,WITH_COVERAGE=False,TINYSPLINE_BUILD_EXAMPLES=False)
        self.flags['WITH_RHI_RENDERER']=False
        self.build=self.root/'graph/build';library_graph(self.build,self.flags)

    def json_edit(self,path,change):
        value=json.loads(path.read_text());change(value);path.write_text(json.dumps(value))

    def test_each_library_profile_owns_one_build_epoch_across_all_stages(self):
        epochs=[1700000000+i for i in range(len(runner.LIBRARY_PROFILES))]
        with patch.dict(os.environ,SOURCE_DATE_EPOCH='poison'),patch.object(runner.time,'time',side_effect=epochs), \
                patch.object(runner,'run_command',side_effect=self.command):
            records=runner.build_library_matrix(self.args,self.selected,{},self.root/'epochs')
        for record,epoch in zip(records,epochs):
            commands=[(c,e) for c,e in self.commands if any('/library-matrix/'+record['profile']+'/' in str(v).replace('\\','/') for v in c)]
            self.assertGreater(len(commands),2)
            self.assertEqual({str(epoch)},{e['SOURCE_DATE_EPOCH'] for c,e in commands})

    def test_python_copy_target_is_required_even_in_gui_less_profiles(self):
        build=self.root/'python-utility/build';flags=dict(self.flags,WITH_PYTHONSCRIPT=True)
        library_graph(build,flags)
        self.assertIn('lcuipy_build_tree',runner.library_profile_graph(build,flags)['targets'])
        self.json_edit(build/'.cmake/api/v1/reply/model.json',lambda v:v['configurations'][1].update(
            targets=[t for t in v['configurations'][1]['targets'] if t['name']!='lcuipy_build_tree']))
        with self.assertRaisesRegex(ValueError,'target mismatch: lcuipy_build_tree'):
            runner.library_profile_graph(build,flags)

    def test_real_graph_parser_requires_owned_release_configuration_and_factory(self):
        self.assertIsNone(runner.library_profile_graph(self.build,self.flags)['legacy_factory_owner'])
        for legacy,rhi in ((True,False),(True,True),(False,True)):
            other=self.root/(str(legacy)+str(rhi))/'build';flags=dict(self.flags,WITH_LEGACY_OPENGL_RENDERER=legacy,WITH_RHI_RENDERER=rhi)
            library_graph(other,flags);actual=runner.library_profile_graph(other,flags)
            self.assertEqual('lcviewer_opengl' if legacy else None,actual['legacy_factory_owner'])

    def test_cold_profiles_require_exact_private_caches_and_explicit_opt_in(self):
        for tests in (True,False):
            flags=dict(self.flags,WITH_UNITTESTS=tests);build=self.root/('pch-'+str(tests))/'build'
            library_graph(build,flags);reply=build/'.cmake/api/v1/reply'
            expected=set(runner.SOURCE_PRIVATE_PCH)-({'lcunittest'} if not tests else set())
            self.assertEqual(expected,set(runner.library_profile_graph(build,flags)['private_pch']))
            cache=build/'CMakeCache.txt';original=cache.read_text()
            for marker in ('','LC_RENDERER_SOURCE_PCH:BOOL=OFF\n'):
                cache.write_text(original.replace('LC_RENDERER_SOURCE_PCH:BOOL=ON\n',marker))
                with self.subTest(tests=tests,marker=marker),self.assertRaisesRegex(ValueError,'explicit private PCH opt-in'):
                    runner.library_profile_graph(build,flags)
            cache.write_text(original)
            for name in expected:
                path=reply/(name+'.json');original=path.read_text()
                self.json_edit(path,lambda d:d['compileGroups'][0].pop('precompileHeaders'))
                with self.subTest(tests=tests,target=name),self.assertRaisesRegex(ValueError,'private source PCH'):
                    runner.library_profile_graph(build,flags)
                path.write_text(original)
            path=reply/('lcviewer_cpu_headercheck.json' if tests else 'lcscripting.json')
            self.json_edit(path,lambda d:d.update(compileGroups=[dict(language='CXX',precompileHeaders=[dict(header='<Eigen/Dense>')])]))
            with self.subTest(tests=tests,leaked=path.name),self.assertRaisesRegex(ValueError,'leaked into another target'):
                runner.library_profile_graph(build,flags)

    def test_foreign_models_ambiguous_release_and_escaping_references_rejected(self):
        reply=self.build/'.cmake/api/v1/reply';model=reply/'model.json';original=model.read_text()
        for change in (lambda v:v['paths'].update(source='/foreign'),lambda v:v['paths'].update(build='/foreign'),
                       lambda v:v['configurations'].append(copy.deepcopy(v['configurations'][1])),
                       lambda v:v['configurations'][1]['targets'][0].update(jsonFile='../foreign.json'),
                       lambda v:v['configurations'][1]['targets'][0].update(name='other'),
                       lambda v:v['configurations'][1]['targets'].append(copy.deepcopy(v['configurations'][1]['targets'][0]))):
            model.write_text(original);self.json_edit(model,change)
            with self.assertRaises(ValueError):runner.source_targets(self.build)

    def test_visual_studio_generator_utilities_are_id_bound_not_application_duplicates(self):
        reply=self.build/'.cmake/api/v1/reply';path=reply/'model.json';model=json.loads(path.read_text())
        for suffix in ('root','nested'):
            name='utility-'+suffix;identifier='ALL_BUILD::'+suffix
            (reply/(name+'.json')).write_text(json.dumps(dict(name='ALL_BUILD',id=identifier,
                type='UTILITY',isGeneratorProvided=True,sources=[])))
            model['configurations'][1]['targets'].append(dict(name='ALL_BUILD',id=identifier,jsonFile=name+'.json'))
        path.write_text(json.dumps(model))
        self.assertNotIn('ALL_BUILD',runner.source_targets(self.build))
        item=reply/'utility-nested.json';original=item.read_text()
        for change in (lambda d:d.update(id='ALL_BUILD::root'),lambda d:d.update(type='SHARED_LIBRARY'),
                       lambda d:d.update(name='lckernel'),lambda d:d.update(isGeneratorProvided=False),
                       lambda d:d.update(artifacts=[dict(path='foreign.dll')])):
            data=json.loads(original);change(data);item.write_text(json.dumps(data))
            with self.subTest(data=data),self.assertRaises(ValueError):runner.source_targets(self.build)
        item.write_text(original)

    def test_qt_ninja_autogen_utilities_remain_visible_and_require_an_owner(self):
        reply=self.build/'.cmake/api/v1/reply';path=reply/'model.json';model=json.loads(path.read_text())
        for suffix in ('_autogen','_autogen_timestamp_deps'):
            name='lckernel'+suffix
            (reply/(name+'.json')).write_text(json.dumps(dict(name=name,id=name+'::fixture',
                type='UTILITY',isGeneratorProvided=True,sources=[dict(path='generated.rule',isGenerated=True)])))
            model['configurations'][1]['targets'].append(dict(name=name,id=name+'::fixture',jsonFile=name+'.json'))
        path.write_text(json.dumps(model))
        self.assertIn('lckernel_autogen',runner.source_targets(self.build))
        self.assertIn('lckernel_autogen_timestamp_deps',runner.source_targets(self.build))
        item=reply/'lckernel_autogen.json';original=item.read_text()
        for change in (lambda d:d.update(type='OBJECT_LIBRARY'),lambda d:d.update(artifacts=[dict(path='foreign.o')]),
                       lambda d:d.update(compileGroups=[dict(language='CXX')]),
                       lambda d:d['sources'][0].update(compileGroupIndex=0)):
            data=json.loads(original);change(data);item.write_text(json.dumps(data))
            with self.subTest(data=data),self.assertRaises(ValueError):runner.source_targets(self.build)
        item.write_text(original)
        model['configurations'][1]['targets']=[r for r in model['configurations'][1]['targets'] if r['name']!='lckernel']
        path.write_text(json.dumps(model))
        with self.assertRaisesRegex(ValueError,'generator-provided'):runner.source_targets(self.build)

    def test_effective_normal_options_and_cache_are_both_required(self):
        preflight=self.build/'renderer-preflight.json';original=preflight.read_text()
        for key in ('WITH_CAIRO','WITH_PERSISTENCE','WITH_RHI_RENDERER','WITH_PYTHONSCRIPT'):
            preflight.write_text(original);self.json_edit(preflight,lambda v:v['options'].update({key:True}))
            with self.assertRaisesRegex(ValueError,'Effective'):runner.library_profile_graph(self.build,self.flags)
        preflight.write_text(original)
        cache=self.build/'CMakeCache.txt';cache.write_text(cache.read_text().replace('WITH_CAIRO:BOOL=OFF','WITH_CAIRO:BOOL=ON'))
        with self.assertRaisesRegex(ValueError,'cache option'):runner.library_profile_graph(self.build,self.flags)

    def test_msvc_optional_debug_symbols_never_replace_required_library_bytes(self):
        target=self.build/'.cmake/api/v1/reply/lckernel.json';data=json.loads(target.read_text())
        binary=self.build/data['artifacts'][0]['path'];binary.parent.mkdir();binary.write_bytes(b'synthetic library')
        data['artifacts'].append(dict(path='lib/lckernel.pdb'));target.write_text(json.dumps(data))
        with patch.object(sys,'platform','win32'):
            self.assertEqual(1,len(runner.library_artifacts(self.build,['lckernel'])['lckernel']))
            pdb=self.build/'lib/lckernel.pdb';pdb.write_bytes(b'synthetic symbols')
            self.assertEqual(2,len(runner.library_artifacts(self.build,['lckernel'])['lckernel']))
            binary.unlink()
            with self.assertRaises(ValueError):runner.library_artifacts(self.build,['lckernel'])
            data['artifacts']=data['artifacts'][1:];target.write_text(json.dumps(data))
            with self.assertRaises(ValueError):runner.library_artifacts(self.build,['lckernel'])

    def test_missing_target_and_legacy_factory_in_cpu_target_rejected(self):
        reply=self.build/'.cmake/api/v1/reply';model=reply/'model.json';original=model.read_text()
        self.json_edit(model,lambda v:v['configurations'][1]['targets'].pop())
        with self.assertRaisesRegex(ValueError,'target mismatch'):runner.library_profile_graph(self.build,self.flags)
        model.write_text(original)
        self.json_edit(reply/'lcviewernoqt.json',lambda v:v['sources'].append(dict(path='painters/opengl/createpainter.cpp')))
        with self.assertRaisesRegex(ValueError,'factory leaked'):runner.library_profile_graph(self.build,self.flags)

    def test_forbidden_discovery_cannot_be_hidden_in_successful_configure(self):
        trace=self.build.parent/'trace.jsonl';original=trace.read_text()
        for package in ('Qt6','Qt6Widgets','Qt5Core','GLEW','glfw3','glm','OpenGL','libdxfrw','Python','Python3','PythonInterp'):
            trace.write_text(original+json.dumps(dict(cmd='find_package',args=[package]))+'\n')
            with self.subTest(package=package),self.assertRaisesRegex(ValueError,'Forbidden'):runner.library_profile_graph(self.build,self.flags)

    def command(self,command,where,timeout,env):
        SourceBuildTests.command(self,command,where,timeout,env)
        if where.name=='configure' and '--trace-format=json-v1' in command:
            tree=Path(command[command.index('-B')+1])
            if getattr(self,'change_provider',False) and where.parent.name=='dual-python':
                path=tree/'python-runtime-inventory.txt';path.write_text(path.read_text().replace('3.12.10','3.12.11'))
        if where.parent.name in runner.LIBRARY_PROFILES and where.name in ('cpu-discovery','cpu-tests'):
            legacy,_,python=runner.LIBRARY_PROFILES[where.parent.name]
            ids=['S.A','S.B']
            if python and not getattr(self,'omit_python',False):
                ids+=['PythonRuntime.IsolatedProviderIgnoresAmbientPaths','PythonRuntime.RuntimeConfigurationIsValidatedInFreshProcesses']
                if getattr(self,'omit_probe',False):ids.pop()
                if getattr(self,'lose_cpu',False):ids.remove('S.B')
            if (legacy and not getattr(self,'omit_backend',False)) or getattr(self,'leak_backend',False):ids+=sorted(runner.LEGACY_TEST_IDS)
            if getattr(self,'subset',False) and where.parent.name=='legacy':ids.remove('S.B')
            if getattr(self,'drift',False) and where.parent.name=='dual-python':ids+=['PythonExtra.OnlyOneMode']
            if where.name=='cpu-discovery':
                listing=''.join(g+'.\n'+''.join('  '+i.split('.')[1]+'\n' for i in ids if i.startswith(g+'.')) for g in sorted({i.split('.')[0] for i in ids}))
                (where/'stdout.log').write_text(listing)
            else:
                xml=ET.Element('testsuites',tests=str(len(ids)),failures='0',disabled='0',errors='0')
                for group in sorted({i.split('.')[0] for i in ids}):
                    names=[i.split('.')[1] for i in ids if i.startswith(group+'.')]
                    suite=ET.SubElement(xml,'testsuite',name=group,tests=str(len(names)),failures='0',disabled='0',errors='0',skipped='0')
                    for name in names:ET.SubElement(suite,'testcase',name=name,classname=group,status='run',result='completed')
                ET.ElementTree(xml).write(where/'tests.xml')
        return dict(synthetic_only=True)

    def execute(self):
        with patch.object(runner,'rhi_selection',return_value=self.selected),patch.object(runner,'run_command',side_effect=self.command):
            return runner.build_rhi_source(self.args)

    def test_all_four_modes_keep_identical_complete_cpu_ids_and_separate_claims(self):
        self.args.case_cmake_option=['dual:CMAKE_TOOLCHAIN_FILE=/sdk/dual']
        directory=self.execute();record=json.loads((directory/'source-build.json').read_text())
        self.assertEqual(list(runner.LIBRARY_PROFILES),[r['profile'] for r in record['library_matrix']])
        self.assertEqual(78,len(self.commands))
        for r in record['library_matrix']:
            self.assertEqual(['S.A','S.B'],r['base_cpu_ids']);self.assertIs(r['phase_or_platform_accepted'],False)
            self.assertEqual(2 if r['options']['WITH_PYTHONSCRIPT'] else 0,len(r['python_ids']))
            self.assertEqual(sorted(runner.LEGACY_TEST_IDS) if r['options']['WITH_LEGACY_OPENGL_RENDERER'] else [],r['backend_ids'])
            self.assertEqual(set(r['base_cpu_ids'])|set(r['python_ids'])|set(r['backend_ids']),set(r['cpu_ids']))
            self.assertIs(r['options']['WITH_RENDERER_TESTS'],False)
            self.assertEqual(1200,r['profile_budget_seconds']);self.assertEqual(self.args.timeout,r['command_timeout_seconds'])
            self.assertEqual(set(runner.SOURCE_PRIVATE_PCH),set(r['observed']['private_pch']))
        configure=[c for c,e in self.commands if '-S' in c]
        self.assertTrue(all('-DLC_RENDERER_SOURCE_PCH:BOOL=ON' in c for c in configure[:-1]))
        self.assertIn('-DCMAKE_TOOLCHAIN_FILE=/sdk/dual',configure[2])
        self.assertNotIn('-DCMAKE_TOOLCHAIN_FILE=/sdk/dual',configure[3])
        for c,e in self.commands:self.assertNotIn('GTEST_FILTER',e)

    def test_library_profile_budget_does_not_expand_command_caps_or_restart_deadline(self):
        timeouts=[];clock=iter(range(0,100000,35))
        def command(argv,where,timeout,env):
            timeouts.append(timeout);return self.command(argv,where,timeout,env)
        with patch.object(runner.time,'monotonic',side_effect=lambda:next(clock)),patch.object(runner,'run_command',side_effect=command):
            records=runner.build_library_matrix(self.args,self.selected,{},self.root/'budget')
        self.assertEqual(len(runner.LIBRARY_PROFILES),len(records))
        self.assertTrue(all(0<t<=self.args.timeout for t in timeouts))
        with patch.object(runner.time,'monotonic',side_effect=[0,0,1201]),patch.object(runner,'run_command',side_effect=self.command):
            with self.assertRaisesRegex(ValueError,'deadline expired'):
                runner.build_library_matrix(self.args,self.selected,{},self.root/'expired')

    def test_intel_kernel_chunks_keep_command_caps_total_deadline_and_normal_link(self):
        self.args.timeout=300;labels=[];timeouts=[]
        def command(argv,where,timeout,env):
            labels.append(where.name);timeouts.append(timeout)
            result=self.command(argv,where,timeout,env)
            if where.name=='configure':
                with (where.parent/'build/CMakeCache.txt').open('a') as out:out.write('CMAKE_GENERATOR:INTERNAL=Ninja\n')
            return result
        clock=iter([0,0,0,100,450,800,1201])
        with patch.object(sys,'platform','darwin'),patch.object(runner.platform,'machine',return_value='x86_64'), \
                patch.object(runner,'kernel_object_targets',return_value=['owned'+str(i)+'.o' for i in range(65)]), \
                patch.object(runner.time,'monotonic',side_effect=lambda:next(clock)), \
                patch.object(runner,'run_command',side_effect=command):
            with self.assertRaisesRegex(ValueError,'Library build deadline expired'):
                runner.build_library_matrix(self.args,self.selected,{},self.root/'intel-budget')
        self.assertEqual(['configure','kernel-part-1','kernel-part-2','kernel-part-3','compile-lckernel'],labels)
        self.assertTrue(all(0<timeout<=300 for timeout in timeouts))
        self.assertEqual('lckernel',self.commands[-1][0][self.commands[-1][0].index('--target')+1])

    def test_profile_subsets_and_rhi_cpu_mismatch_rejected(self):
        self.subset=True
        with self.assertRaises(ValueError):self.execute()
        self.subset=False
        with patch.object(runner,'build_library_matrix',return_value=[dict(cpu_ids=['S.A'])]):
            with self.assertRaisesRegex(ValueError,'different CPU IDs'):self.execute()

    def test_required_library_groups_cover_all_profiles_and_keep_exact_cpu_parity(self):
        groups={}
        for group in ('core','python'):
            self.args.app=None;self.args.build_dir=None
            self.args.library_group=group;directory=self.execute()
            record=json.loads((directory/'source-build.json').read_text())
            groups[group]={r['profile'] for r in record['library_matrix']}
            self.assertEqual(group,record['library_group'])
            for r in record['library_matrix']:
                self.assertEqual(group,r['library_group']);self.assertEqual(record['cpu_executed_ids'],r['base_cpu_ids'])
                self.assertEqual(1200,r['profile_budget_seconds'])
        self.assertEqual({'cpu','legacy','dual'},groups['core'])
        self.assertEqual({'cpu','cpu-python','legacy-python','dual-python','rhi-python'},groups['python'])
        self.assertEqual(set(runner.LIBRARY_PROFILES),groups['core']|groups['python'])
        self.assertEqual({'cpu'},groups['core']&groups['python'])
        self.args.app=None;self.args.build_dir=None;self.omit_probe=True
        with self.assertRaises(ValueError):self.execute()

    def test_tests_on_matrix_retains_and_closes_the_full_cpu_source_map(self):
        original={'lcviewernoqt/painters/hatchrecorder.cpp':'before'}
        with patch.object(runner,'production_inputs',return_value=original),patch.object(runner,'run_command',side_effect=self.command):
            records=runner.build_library_matrix(self.args,self.selected,{},self.root/'source-map')
        self.assertTrue(records)
        self.assertTrue(all(record['source_sha256']==original for record in records))
        with patch.object(runner,'production_inputs',side_effect=[original,{**original,next(iter(original)):'changed'}]), \
                patch.object(runner,'run_command',side_effect=self.command):
            with self.assertRaisesRegex(ValueError,'Library matrix source/helper changed'):
                runner.build_library_matrix(self.args,self.selected,{},self.root/'source-drift')

    def test_profile_sdk_overrides_cannot_change_owned_flags_or_select_subset(self):
        for values in (['rhi:Qt6_DIR=/sdk'],['dual:WITH_UNITTESTS=OFF'],['cpu:Qt6_DIR=/one','cpu:Qt6_DIR=/two']):
            self.args.case_cmake_option=values
            with self.subTest(values=values),self.assertRaises(ValueError):self.execute()
        self.assertEqual([],self.commands)

    def test_python_source_membership_and_provider_receipt_are_required(self):
        flags=dict(self.flags,WITH_PYTHONSCRIPT=True);build=self.root/'python-case/build';library_graph(build,flags)
        self.assertEqual('CPython',runner.library_profile_graph(build,flags)['python_provider']['provider_implementation'])
        target=build/'.cmake/api/v1/reply/lcunittest.json';original=target.read_text()
        self.json_edit(target,lambda v:v.update(sources=[s for s in v['sources'] if s['path']!=runner.PYTHON_TEST_SOURCES[-1]]))
        with self.assertRaisesRegex(ValueError,'test-source membership'):runner.library_profile_graph(build,flags)
        target.write_text(original);path=build/'python-runtime-inventory.txt';raw=path.read_text()
        for changed in (raw+'executable='+sys.executable+'\n',raw.replace('CPython','PyPy'),raw.replace('provider_pointer_bits=64','provider_pointer_bits=32'),
                        raw.replace('provider_pointer_bits=64','target_pointer_bits=64')):
            path.write_text(changed)
            with self.assertRaises(ValueError):runner.library_profile_graph(build,flags)

    def test_python_inventory_uses_the_real_cmake_producer_schema(self):
        flags=dict(self.flags,WITH_PYTHONSCRIPT=True);build=self.root/'producer-python/build';library_graph(build,flags)
        source=(inventory.REPO_ROOT/'lcadpythonscript/CMakeLists.txt').read_text()
        # Execute the producer's own bounded identity query rather than inventing
        # an ABI field in the fixture. Only paths/headers remain synthetic here.
        query=re.search(r'"(import platform,struct,sys,sysconfig;[^"\n]+)"',source)
        self.assertIsNotNone(query)
        actual=subprocess.run([sys.executable,'-I','-S','-c',query[1]],capture_output=True,text=True,check=True,timeout=10).stdout
        path=build/'python-runtime-inventory.txt';raw=path.read_text()
        path.write_text(''.join(line+'\n' for line in raw.splitlines() if not line.startswith('provider_'))+actual)
        selected=runner.library_profile_graph(build,flags)['python_provider']
        self.assertEqual('64',selected['provider_pointer_bits']);self.assertNotIn('target_pointer_bits',selected)

    def test_backend_test_inventory_is_exact_and_bound_to_its_real_source(self):
        source=(inventory.REPO_ROOT/'unittest/lcviewernoqt/fontbooktest.cpp').read_text()
        self.assertEqual(runner.LEGACY_TEST_IDS,{suite+'.'+name for suite,name in re.findall(r'^TEST\((\w+),\s*(\w+)\)',source,re.M)})
        for flag in ('omit_backend','leak_backend'):
            setattr(self,flag,True)
            with self.subTest(flag=flag),self.assertRaisesRegex(ValueError,'legacy tests'):self.execute()
            setattr(self,flag,False)
        flags=dict(self.flags,WITH_LEGACY_OPENGL_RENDERER=True);build=self.root/'legacy-membership/build';library_graph(build,flags)
        path=build/'.cmake/api/v1/reply/lcunittest.json'
        self.json_edit(path,lambda target:target.update(sources=[s for s in target['sources']
            if not s['path'].endswith('lcviewernoqt/fontbooktest.cpp')]))
        with self.assertRaisesRegex(ValueError,'Legacy test-source membership'):runner.library_profile_graph(build,flags)

    def test_windows_python_empty_soabi_requires_exact_extension_abi(self):
        flags=dict(self.flags,WITH_PYTHONSCRIPT=True);build=self.root/'windows-python/build';library_graph(build,flags)
        path=build/'python-runtime-inventory.txt'
        raw=path.read_text().replace('provider_soabi=synthetic-only','provider_soabi=')+(
            'provider_platform=win-amd64\nprovider_extension_suffix=.cp312-win_amd64.pyd\n')
        path.write_text(raw)
        with patch.object(sys,'platform','win32'):
            self.assertEqual('',runner.library_profile_graph(build,flags)['python_provider']['provider_soabi'])
            for changed in (raw.replace('win-amd64','win32'),raw.replace('.cp312-win_amd64.pyd','.cp313-win_amd64.pyd'),
                            raw.replace('3.12.10','3.13.1'),raw.replace('provider_soabi=\n',''),
                            raw.replace('provider_extension_suffix=.cp312-win_amd64.pyd\n','')):
                path.write_text(changed)
                with self.subTest(inventory=changed),self.assertRaisesRegex(ValueError,'provider ABI'):
                    runner.library_profile_graph(build,flags)
        path.write_text(raw)
        with patch.object(sys,'platform','linux'),self.assertRaisesRegex(ValueError,'provider ABI'):
            runner.library_profile_graph(build,flags)

    def test_python_cannot_drop_core_tests_or_differ_between_renderers(self):
        for name in ('omit_python','omit_probe','lose_cpu','drift','change_provider'):
            setattr(self,name,True)
            with self.subTest(name=name),self.assertRaises(ValueError):self.execute()
            setattr(self,name,False)

    def test_explicit_python_sdk_cannot_silently_select_another_interpreter(self):
        self.args.cmake_option=['Python_EXECUTABLE=/foreign/python']
        with self.assertRaisesRegex(ValueError,'requested SDK'):self.execute()

    def test_python_off_must_not_leave_a_provider_inventory(self):
        (self.build/'python-runtime-inventory.txt').write_text('not a selected provider')
        with self.assertRaisesRegex(ValueError,'Python OFF'):runner.library_profile_graph(self.build,self.flags)


class ProductionBuildTests(unittest.TestCase):
    def setUp(self):
        SourceBuildTests.setUp(self);self.args.platform='MAC-ARM64';self.args.configuration=None
    def command(self,command,where,timeout,env):
        result=SourceBuildTests.command(self,command,where,timeout,env)
        if where.name=='configure' and where.parent.name=='dual-python' and getattr(self,'provider_drift',False):
            path=where.parent/'build/python-runtime-inventory.txt';path.write_text(path.read_text().replace('3.12.10','3.12.11'))
        if where.name.startswith('compile-') and where.parent.name=='rhi' and getattr(self,'artifact_drift',False):
            (where.parent.parent/'cpu/build/lib/lckernel.fixture').write_bytes(b'changed after successful compilation')
        return result

    def execute(self):
        with patch.object(runner,'native_platform',return_value='MAC-ARM64'),patch.object(runner,'run_command',side_effect=self.command):
            return runner.build_production_libraries(self.args)

    def test_all_eight_modes_compile_without_gtest_or_runtime_claims(self):
        directory=self.execute();record=json.loads((directory/'production-build.json').read_text())
        self.assertIs(record['phase_or_platform_accepted'],False);self.assertIs(record['runtime_tests_executed'],False)
        self.assertEqual(set(runner.LIBRARY_PROFILES)|{'rhi'},{r['profile'] for r in record['profiles']})
        self.assertEqual(52,len(self.commands))
        for item in record['profiles']:
            self.assertIs(item['options']['WITH_UNITTESTS'],False);self.assertIsNone(item['observed']['cpu_headers'])
            self.assertIs(item['runtime_tests_executed'],False);self.assertNotIn('cpu_ids',item)
            self.assertGreaterEqual(len(item['artifacts_sha256']),4)
            self.assertEqual(set(runner.SOURCE_PRIVATE_PCH)-{'lcunittest'},set(item['observed']['private_pch']))
            for target in ('lcunittest','lcbuildcontracttest','lcviewer_cpu_headercheck','lcrenderertest','lcrenderer_headercheck'):
                self.assertNotIn(target,item['observed']['targets'])
        for command,env in self.commands:
            self.assertNotIn('GTEST_FILTER',env);self.assertNotIn('--gtest_list_tests',command)
            if '-S' in command:self.assertIn('-DCMAKE_DISABLE_FIND_PACKAGE_GTest=ON',command)

    def test_required_compile_only_groups_partition_all_eight_modes_without_tests(self):
        groups={}
        for group in ('core','python'):
            self.args.library_group=group;directory=self.execute()
            record=json.loads((directory/'production-build.json').read_text());self.assertEqual(group,record['library_group'])
            groups[group]={r['profile'] for r in record['profiles']}
            self.assertEqual(4,len(groups[group]))
            for r in record['profiles']:
                self.assertEqual(group=='python',r['options']['WITH_PYTHONSCRIPT'])
                self.assertFalse(r['runtime_tests_executed']);self.assertEqual(group,r['library_group'])
        self.assertFalse(groups['core']&groups['python'])
        self.assertEqual(set(runner.LIBRARY_PROFILES)|{'rhi'},groups['core']|groups['python'])
        self.args.library_group='missing';self.commands=[]
        with self.assertRaisesRegex(ValueError,'library group'):self.execute()
        self.assertEqual([],self.commands)

    def test_project_test_targets_and_gtest_discovery_are_forbidden(self):
        directory=self.execute();build=directory/'library-matrix/cpu/build'
        flags=json.loads((directory/'library-matrix/cpu/library-build.json').read_text())['options']
        model=build/'.cmake/api/v1/reply/model.json';original=model.read_text()
        for target in ('lcunittest','lcbuildcontracttest','lcviewer_cpu_headercheck','lcrenderertest','lcrenderer_headercheck'):
            value=json.loads(original);value['configurations'][1]['targets'].append(dict(name=target,id=target+'::injected',jsonFile='injected.json'))
            (model.parent/'injected.json').write_text(json.dumps(dict(name=target,id=target+'::injected')))
            model.write_text(json.dumps(value))
            with self.subTest(target=target),self.assertRaisesRegex(ValueError,'target mismatch'):runner.library_profile_graph(build,flags)
        model.write_text(original);trace=build.parent/'trace.jsonl';original=trace.read_text()
        for package in ('GTest','GMock','googletest'):
            trace.write_text(original+json.dumps(dict(cmd='find_package',args=[package]))+'\n')
            with self.subTest(package=package),self.assertRaisesRegex(ValueError,'Forbidden'):runner.library_profile_graph(build,flags)

    def test_artifacts_must_be_owned_nonempty_unique_library_outputs(self):
        directory=self.execute();build=directory/'library-matrix/cpu/build';target=build/'.cmake/api/v1/reply/lckernel.json'
        original=json.loads(target.read_text())
        for change in (lambda v:v.update(type='UTILITY'),lambda v:v.update(artifacts=[]),
                       lambda v:v.update(artifacts=[dict(path='/outside/library.a')]),
                       lambda v:v['artifacts'].append(v['artifacts'][0])):
            value=copy.deepcopy(original);change(value);target.write_text(json.dumps(value))
            with self.assertRaises(ValueError):runner.library_artifacts(build,['lckernel'])
        target.write_text(json.dumps(original));(build/original['artifacts'][0]['path']).write_bytes(b'')
        with self.assertRaises(ValueError):runner.library_artifacts(build,['lckernel'])

    def test_source_drift_and_requested_sdk_mismatch_fail(self):
        with patch.object(runner,'production_inputs',side_effect=[{'fixture':'before'},{'fixture':'changed'}]):
            with self.assertRaisesRegex(ValueError,'source/helper changed'):self.execute()
        self.args.cmake_option=['Python_EXECUTABLE=/foreign/python']
        with self.assertRaisesRegex(ValueError,'requested SDK'):self.execute()

    def test_provider_and_prior_profile_artifact_drift_invalidate_the_matrix(self):
        for name in ('provider_drift','artifact_drift'):
            setattr(self,name,True)
            with self.subTest(name=name),self.assertRaisesRegex(ValueError,'different providers|inputs/artifacts changed'):self.execute()
            setattr(self,name,False)

    def test_native_platform_reuse_and_fixture_overrides_fail_before_compilation(self):
        for key,value in [('platform','WIN-X64'),('app',Path('/reuse/app')),('build_dir',Path('/reuse/build')),
                          ('configuration','cpu-source/MAC-ARM64/cpu/cpu/none/none'),('cmake_option',['WITH_UNITTESTS=ON']),
                          ('case_cmake_option',['not-a-profile:Qt6_DIR=/sdk'])]:
            old=getattr(self.args,key,None);setattr(self.args,key,value)
            with self.subTest(key=key),self.assertRaises(ValueError):self.execute()
            setattr(self.args,key,old)
        self.assertEqual([],self.commands)

    def test_cli_cannot_alias_a_graphics_or_test_manifest_run(self):
        for flags in ([],['--platform','MAC-ARM64','--backend','metal'],['--platform','MAC-ARM64','--rhi-contracts']):
            result=subprocess.run([sys.executable,'-B',runner.__file__,'--production-builds',*flags,
                '--report-dir',str(self.root)],capture_output=True,timeout=10)
            self.assertEqual(2,result.returncode)


class SourceCheckGraphTests(unittest.TestCase):
    required=('lcrenderertest','lcrenderer_headercheck','lcviewer_cpu_headercheck','lcunittest')

    def setUp(self):
        temporary=tempfile.TemporaryDirectory(prefix='renderer-source-check-graph-');self.addCleanup(temporary.cleanup)
        self.build=Path(temporary.name)/'build'
        library_graph(self.build,dict(WITH_UNITTESTS=True,WITH_RENDERER_TESTS=True,WITH_RHI_RENDERER=True,
            WITH_LEGACY_OPENGL_RENDERER=False,WITH_PYTHONSCRIPT=False))
        self.reply=self.build/'.cmake/api/v1/reply'
        self.path=self.reply/'lcrenderer_source_checks.json';self.original=json.loads(self.path.read_text())

    def check(self):return runner.source_check_graph(self.build)

    def write(self,value):self.path.write_text(json.dumps(value))

    def test_owned_aggregate_keeps_all_four_exact_direct_targets(self):
        graph=self.check()
        expected={name:name+'::fixture' for name in self.required}
        self.assertEqual('lcrenderer_source_checks::fixture',graph['target'])
        self.assertEqual(expected,graph['required_targets'])
        # The libraries build at the project's standard; only the test runner pins its own.
        project=runner.project_cxx_standard()
        expected={'lckernel':(project,['<Eigen/Dense>']),'lcviewernoqt':(project,['<Eigen/Dense>']),
            'lcluascript':(project,['<Eigen/Dense>','<kaguya/kaguya.hpp>']),
            'lcunittest':('17',['<Eigen/Dense>','<gtest/gtest.h>'])}
        self.assertEqual({name:dict(target=name+'::fixture',standard=standard,headers=headers,private=True)
            for name,(standard,headers) in expected.items()},graph['private_pch'])

    def test_private_pch_requires_exact_ordered_headers_in_every_target_group(self):
        mutations=(lambda d:d.update(type='OBJECT_LIBRARY'),lambda d:d.update(compileGroups=[]),
            lambda d:d['compileGroups'][0].update(language='C'),
            lambda d:d['compileGroups'][0]['languageStandard'].update(standard='20'),
            lambda d:d['compileGroups'][0].pop('precompileHeaders'),
            lambda d:d['compileGroups'][0].update(precompileHeaders=None),
            lambda d:d['compileGroups'][0].update(precompileHeaders=['<Eigen/Dense>']),
            lambda d:d['compileGroups'][0].update(precompileHeaders=[]),
            lambda d:d['compileGroups'][0].update(precompileHeaders=[dict(header='<Eigen/Core>')]),
            lambda d:d['compileGroups'][0]['precompileHeaders'].append(dict(header='cad/logger/logger.h')),
            lambda d:d['compileGroups'][0]['precompileHeaders'].reverse())
        for name in runner.SOURCE_PRIVATE_PCH:
            path=self.reply/(name+'.json');original=json.loads(path.read_text())
            for mutate in mutations:
                value=copy.deepcopy(original);mutate(value)
                if value==original:continue
                path.write_text(json.dumps(value))
                with self.subTest(target=name,mutate=mutate),self.assertRaisesRegex(ValueError,'private source PCH'):self.check()
            path.write_text(json.dumps(original));path.unlink()
            with self.subTest(missing=name),self.assertRaises((ValueError,FileNotFoundError)):self.check()
            path.write_text(json.dumps(original))

    def test_private_pch_cannot_leak_to_headers_or_other_consumers(self):
        for name in ('lcviewer_cpu_headercheck','lcrenderer_headercheck','lcrenderertest','lcscripting','lcrenderer_rhi'):
            path=self.reply/(name+'.json');original=path.read_bytes()
            for mutation in ('metadata','flags','source'):
                value=json.loads(original)
                group=value.setdefault('compileGroups',[dict(language='CXX')])[0]
                if mutation=='metadata':group['precompileHeaders']=[dict(header='<Eigen/Dense>')]
                if mutation=='flags':group['compileCommandFragments']=[dict(fragment='-include /foreign/cmake_pch.hxx')]
                if mutation=='source':value['sources']=[dict(path='/foreign/cmake_pch.hxx')]
                path.write_text(json.dumps(value))
                with self.subTest(target=name,mutation=mutation),self.assertRaisesRegex(ValueError,'leaked'):self.check()
            path.write_bytes(original)

    def test_cmake_pch_is_private_owned_and_explicitly_opted_in(self):
        source=(inventory.REPO_ROOT/'CMakeLists.txt').read_text()
        block='option(LC_RENDERER_SOURCE_PCH '+source.split('option(LC_RENDERER_SOURCE_PCH ',1)[1].split(
            'if(WITH_RENDERER_TESTS)\n    # One graph',1)[0]
        self.assertIn('"Private header caches for cold renderer source checks" OFF)',block)
        self.assertIn('if(WITH_RENDERER_TESTS OR LC_RENDERER_SOURCE_PCH)',block)
        self.assertIn('if(TARGET ${_target})',block);self.assertIn('if(TARGET lcunittest)',block)
        self.assertNotIn('REUSE_FROM',block)
        self.assertEqual(3,source.count('target_precompile_headers('))
        self.assertNotIn('target_precompile_headers(', (inventory.REPO_ROOT/'lckernel/CMakeLists.txt').read_text())
        directory=self.build.parent/'scope';directory.mkdir();(directory/'dummy.cpp').write_text('int fixture() { return 0; }\n')
        for header in ('Eigen/Dense','gtest/gtest.h','kaguya/kaguya.hpp'):
            path=directory/'sdk'/header;path.parent.mkdir(parents=True,exist_ok=True);path.write_text('#pragma once\n')
        for tests,renderer,cold in ((True,False,False),(False,False,False),(True,True,False),
                                    (True,False,True),(False,False,True)):
            with self.subTest(tests=tests,renderer=renderer,cold=cold):
                names=[n for n in runner.SOURCE_PRIVATE_PCH if n!='lcunittest' or tests]+['headercheck','lcscripting']
                (directory/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.21)\nproject(PchScope LANGUAGES CXX)\n'
                    +''.join('add_library('+name+' STATIC dummy.cpp)\ntarget_include_directories('+name+' PRIVATE sdk)\n' for name in names)+block+'\n'
                    'foreach(name IN ITEMS '+' '.join(names)+')\n'
                    'get_target_property(own ${name} PRECOMPILE_HEADERS)\n'
                    'get_target_property(exported ${name} INTERFACE_PRECOMPILE_HEADERS)\n'
                    'get_target_property(reused ${name} PRECOMPILE_HEADERS_REUSE_FROM)\n'
                    'file(APPEND "${CMAKE_BINARY_DIR}/scope.txt" "${name}\\n${own}\\n${exported}\\n${reused}\\n")\nendforeach()\n')
                build=directory/(str(tests)+str(renderer)+str(cold))
                options=['-DWITH_RENDERER_TESTS='+('ON' if renderer else 'OFF')]
                if cold:options+=['-DLC_RENDERER_SOURCE_PCH=ON']
                result=subprocess.run(['cmake','-S',str(directory),'-B',str(build),*options],capture_output=True,text=True,timeout=30)
                self.assertEqual(0,result.returncode,result.stdout+result.stderr)
                self.assertEqual([value for name in names for value in
                    (name,';'.join(runner.SOURCE_PRIVATE_PCH[name][1]) if (renderer or cold) and name in runner.SOURCE_PRIVATE_PCH
                     else 'own-NOTFOUND','exported-NOTFOUND','reused-NOTFOUND')],(build/'scope.txt').read_text().splitlines())
                if cold:
                    result=subprocess.run(['cmake','--build',str(build),'--parallel','4'],capture_output=True,text=True,timeout=30)
                    self.assertEqual(0,result.returncode,result.stdout+result.stderr)

    def pch_files(self):
        result={}
        for name in runner.SOURCE_PRIVATE_PCH:
            data=json.loads((self.reply/(name+'.json')).read_text())
            owner=self.build/data['paths']['build']/'CMakeFiles'/(name+'.dir')
            binary=owner/'cmake_pch.hxx.pch';binary.write_bytes(b'synthetic-pch-not-native')
            result[name]=(owner/'cmake_pch.hxx',owner/'cmake_pch.hxx.cxx',binary)
        return result

    def test_compiled_private_pch_requires_each_owned_declared_input_and_cache(self):
        files=self.pch_files()
        expected={name:dict(generated_sha256={str(p.relative_to(self.build)):runner.sha256(p.read_bytes()) for p in paths[:2]},
            compiled_sha256={str(paths[2].relative_to(self.build)):runner.sha256(paths[2].read_bytes())}) for name,paths in files.items()}
        self.assertEqual(expected,runner.private_pch_contract(self.build))
        for name,(header,source,binary) in files.items():
            saved={p:p.read_bytes() for p in (header,source,binary)}
            target=self.reply/(name+'.json');original=target.read_bytes();extra=header.parent/'cmake_pch.pch'
            def graph(change):
                data=json.loads(original);change(data);target.write_text(json.dumps(data))
            cases=(lambda:header.unlink(),lambda:source.unlink(),lambda:binary.unlink(),lambda:header.write_bytes(b''),
                lambda:binary.write_bytes(b''),lambda:extra.write_bytes(b'duplicate'),
                lambda:graph(lambda d:d.update(sources=[s for s in d['sources'] if s['path']!=str(header)])),
                lambda:graph(lambda d:d['paths'].update(build='../foreign')),
                lambda:header.write_text('#include <Eigen/Core>\n'),
                lambda:header.write_text('// #include <Eigen/Dense>\n/*\n#include <Eigen/Dense>\n*/\n'),
                lambda:header.write_bytes(saved[header]+b'#include <vector>\n'))
            for change in cases:
                change()
                with self.subTest(target=name,change=change),self.assertRaises(ValueError):runner.private_pch_contract(self.build)
                target.write_bytes(original)
                for path,raw in saved.items():path.write_bytes(raw)
                if extra.exists():extra.unlink()

    def test_visual_studio_pch_uses_release_inputs_and_separate_object_directory(self):
        files=self.pch_files();expected={}
        for name,(old_header,old_source,old_binary) in files.items():
            owner=old_header.parent;header=owner/'Release/cmake_pch.hxx';header.parent.mkdir();old_header.rename(header)
            source=owner/'cmake_pch.cxx';old_source.rename(source);old_binary.unlink()
            for configuration in ('Debug','MinSizeRel','RelWithDebInfo'):
                other=owner/configuration/'cmake_pch.hxx';other.parent.mkdir();other.write_bytes(header.read_bytes())
            target=self.reply/(name+'.json');data=json.loads(target.read_text())
            data['sources']=[s for s in data['sources'] if not Path(s['path']).name.startswith('cmake_pch')]
            data['sources'] += [dict(path=str(header)),dict(path=str(source),compileGroupIndex=0)]
            data['sources'] += [dict(path=str(owner/configuration/'cmake_pch.hxx'))
                for configuration in ('Debug','MinSizeRel','RelWithDebInfo')]
            target.write_text(json.dumps(data))
            binary=self.build/data['paths']['build']/(name+'.dir')/'Release/cmake_pch.pch';binary.parent.mkdir(parents=True)
            binary.write_bytes(b'synthetic-visual-studio-pch')
            expected[name]=dict(generated_sha256={str(p.relative_to(self.build)):runner.sha256(p.read_bytes()) for p in (header,source)},
                compiled_sha256={str(binary.relative_to(self.build)):runner.sha256(binary.read_bytes())})
        self.assertEqual(expected,runner.private_pch_contract(self.build))
        for name,receipt in expected.items():
            binary=self.build/next(iter(receipt['compiled_sha256']))
            owner=self.build/json.loads((self.reply/(name+'.json')).read_text())['paths']['build']
            duplicate=owner/'CMakeFiles'/(name+'.dir')/'Release/cmake_pch.pch';duplicate.write_bytes(b'duplicate')
            with self.subTest(target=name),self.assertRaises(ValueError):runner.private_pch_contract(self.build)
            duplicate.unlink();binary.unlink()
            foreign=owner/'foreign.dir/Release/cmake_pch.pch';foreign.parent.mkdir(parents=True)
            foreign.write_bytes(b'foreign-target-pch')
            with self.subTest(foreign=name),self.assertRaises(ValueError):runner.private_pch_contract(self.build)
            foreign.unlink();binary.write_bytes(b'synthetic-visual-studio-pch')

    def test_omitting_any_required_direct_target_is_rejected_even_if_reachable(self):
        for name in self.required:
            graph=copy.deepcopy(self.original)
            graph['dependencies']=[edge for edge in graph['dependencies'] if edge['id']!=name+'::fixture']
            self.write(graph)
            via=next(n for n in self.required if n!=name);path=self.reply/(via+'.json');original=path.read_bytes()
            child=json.loads(original);child['dependencies']=[dict(id=name+'::fixture')];path.write_text(json.dumps(child))
            with self.subTest(target=name),self.assertRaises(ValueError):self.check()
            path.write_bytes(original)

    def test_forged_required_ids_malformed_and_duplicate_dependency_edges_are_rejected(self):
        mutations=[lambda g:g['dependencies'][0].update(id='lcrenderertest::forged'),
            lambda g:g['dependencies'].append(copy.deepcopy(g['dependencies'][0])),
            lambda g:g['dependencies'].append(dict(missing='id'))]
        for mutate in mutations:
            graph=copy.deepcopy(self.original);mutate(graph);self.write(graph)
            with self.subTest(mutate=mutate),self.assertRaises(ValueError):self.check()

    def test_aggregate_cannot_be_generated_compiled_or_publish_an_artifact(self):
        mutations=[lambda g:g.update(type='SHARED_LIBRARY'),lambda g:g.update(isGeneratorProvided=True),
            lambda g:g.update(artifacts=[dict(path='lib/pretend.a')]),
            lambda g:g.update(compileGroups=[dict(language='CXX')]),
            lambda g:g.update(sources=[dict(path='pretend.cpp',compileGroupIndex=0)])]
        for mutate in mutations:
            graph=copy.deepcopy(self.original);mutate(graph);self.write(graph)
            with self.subTest(mutate=mutate),self.assertRaises(ValueError):self.check()


class CpuHeaderContractTests(unittest.TestCase):
    def setUp(self):LibraryMatrixTests.setUp(self)

    def test_independent_configured_consumers_require_real_nonempty_outputs(self):
        result=runner.cpu_header_contract(self.build)
        self.assertEqual('14',result['standard']);self.assertEqual(7,len(result['configured_sources_sha256']))
        with self.assertRaisesRegex(ValueError,'not compiled'):runner.cpu_header_contract(self.build,True)
        for p in (self.build/'lcviewernoqt/cpu-headers/objects-Release.txt').read_text().splitlines():Path(p).write_bytes(b'synthetic-object')
        self.assertEqual(7,len(runner.cpu_header_contract(self.build,True)['objects_sha256']))

    def test_cpp17_other_languages_and_foreign_or_missing_sources_fail(self):
        path=self.build/'.cmake/api/v1/reply/lcviewer_cpu_headercheck.json';original=json.loads(path.read_text())
        for mutate in (lambda d:d.update(type='SHARED_LIBRARY'),lambda d:d['compileGroups'][0]['languageStandard'].update(standard='17'),
                       lambda d:d['compileGroups'][0].update(language='C'),lambda d:d['sources'].pop(),
                       lambda d:d['sources'][0].update(path='/foreign/header.cpp'),
                       lambda d:d['sources'][0].update(compileGroupIndex=True),
                       lambda d:d['sources'].append(copy.deepcopy(d['sources'][0]))):
            value=copy.deepcopy(original);mutate(value);path.write_text(json.dumps(value))
            with self.assertRaises(ValueError):runner.cpu_header_contract(self.build)

    def test_changed_consumer_cannot_remove_its_sdk_or_standard_guard(self):
        directory=self.build/'lcviewernoqt/cpu-headers';path=next(directory.glob('*.cpp'))
        path.write_text(path.read_text().replace('#error CPU public headers','// CPU public headers'))
        with self.assertRaisesRegex(ValueError,'Changed configured'):runner.cpu_header_contract(self.build)

    def test_foreign_duplicate_missing_and_alias_object_lists_fail(self):
        path=self.build/'lcviewernoqt/cpu-headers/objects-Release.txt';original=path.read_text().splitlines()
        cases=[original[:-1],original+[original[0]],['/outside/foreign.o']+original[1:],
            [original[0],original[0],*original[2:]],
            [original[0],str(Path(original[0]).parent/'alias/..'/Path(original[0]).name),*original[2:]]]
        for values in cases:
            path.write_text('\n'.join(values)+'\n')
            with self.subTest(values=values),self.assertRaises(ValueError):runner.cpu_header_contract(self.build)


class SourceWorkflowTests(unittest.TestCase):
    def setUp(self):
        self.text=(inventory.REPO_ROOT/'.github/workflows/renderer-source.yml').read_text()

    def test_ci_source_guard_keeps_owned_sdk_ignored_but_rejects_real_source_dirt(self):
        marker='- name: Require clean source before compilation'
        block=self.text.split(marker,1)[1].split('\n      - name:',1)[0]
        code=textwrap.dedent(block.split('run: |\n',1)[1])
        self.assertLess(self.text.index(marker),self.text.index('- name: Run selected library or CLI source contracts'))
        sdk_block=self.text.split('- name: Stage immutable Vulkan headers without compiling a dependency',1)[1].split('\n      - name:',1)[0]
        sdk=re.search(r'(?m)^\s+path: (\S+)$',sdk_block).group(1)
        self.assertEqual('build-r1/vulkan-headers',sdk)
        self.assertEqual(3,self.text.count("Path('"+sdk+"/include')"))
        with tempfile.TemporaryDirectory(prefix='renderer-ci-clean-source-') as temporary:
            root=Path(temporary);ignore=(inventory.REPO_ROOT/'.gitignore').read_bytes()
            (root/'.gitignore').write_bytes(ignore);(root/'source.cpp').write_bytes(b'original source\n')
            def git(*args,cwd=root):
                result=subprocess.run(['git',*args],cwd=cwd,capture_output=True,text=True,timeout=30)
                self.assertEqual(0,result.returncode,result.stdout+result.stderr)
            git('init','-q');git('add','.gitignore','source.cpp')
            git('-c','user.name=Fixture','-c','user.email=fixture@example.invalid','-c','commit.gpgsign=false','commit','-qm','fixture')
            header=root/sdk/'include/vulkan/vulkan.h';header.parent.mkdir(parents=True);header.write_bytes(b'owned SDK\n')
            git('init','-q',cwd=root/sdk)
            receipt=root/'build-r1/source-status-before-build.log'
            def guard():
                return subprocess.run([sys.executable,'-B','-c',code],cwd=root,capture_output=True,text=True,timeout=30)
            clean=guard();self.assertEqual(0,clean.returncode,clean.stdout+clean.stderr)
            self.assertEqual(b'',receipt.read_bytes())
            for name in ('.r1-vulkan-headers/include/vulkan/vulkan.h','untracked/source.cpp','CMakeUserPresets.json','source.cpp'):
                with self.subTest(path=name):
                    path=root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(b'user bytes\n')
                    if name.startswith('.r1-vulkan-headers/'):git('init','-q',cwd=root/'.r1-vulkan-headers')
                    dirty=guard();self.assertNotEqual(0,dirty.returncode)
                    self.assertIn(('.r1-vulkan-headers/' if name.startswith('.r1-vulkan-headers/') else name).encode(),receipt.read_bytes())
                    self.assertEqual(b'user bytes\n',path.read_bytes())
                    if name=='source.cpp':path.write_bytes(b'original source\n')
                    elif name.startswith('.r1-vulkan-headers/'):shutil.rmtree(root/'.r1-vulkan-headers')
                    else:path.unlink()
            self.assertEqual(ignore,(root/'.gitignore').read_bytes())
            self.assertEqual(b'owned SDK\n',header.read_bytes())
            self.assertIn('build-r1/source-status-before-build.log',self.retained_files(self.text,
                'Retain selected source/provider/build/test/process evidence on failure too',root))

    def retained_files(self,text,step,root):
        block=text.split('- name: '+step,1)[1].split('\n      - ',1)[0]
        self.assertIn('uses: actions/upload-artifact@',block)
        patterns=textwrap.dedent(block.split('          path: |\n',1)[1].split('\n          if-no-files-found:',1)[0]).splitlines()
        # Exercise the actual positive file/directory globs with real files,
        # rather than accepting a token elsewhere in a workflow or comment.
        selected=set()
        for pattern in patterns:
            self.assertTrue(pattern and not pattern.startswith('!'),'retention model needs exclusion support')
            for path in root.glob(pattern):
                files=path.rglob('*') if path.is_dir() else [path]
                selected.update(p.relative_to(root).as_posix() for p in files if p.is_file())
        return selected

    def test_source_uploader_retains_rhi_text_bytes_and_old_filter_drops_them(self):
        with tempfile.TemporaryDirectory(prefix='renderer-source-retention-') as directory:
            root=Path(directory);required=set();reports=set()
            for qpa in ('xcb','wayland','cocoa','windows'):
                base='build-r1/rhi-'+qpa+'/run-owned/'
                required.add(base+'discovery.txt');reports.add(base+'checks.json')
                if qpa in ('xcb','wayland'):required.add(base+'vk_layer_settings.txt')
            unrelated={'build-r1/rhi-xcb/run-owned/unreported.txt','outside/discovery.txt','outside/vk_layer_settings.txt'}
            for name in required|reports|unrelated:
                path=root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(name)
            step='Retain selected source/provider/build/test/process evidence on failure too'
            selected=self.retained_files(self.text,step,root)
            self.assertTrue(required|reports<=selected);self.assertFalse(unrelated&selected)
            old=self.text.replace('            build-r1/**/discovery.txt\n','').replace('            build-r1/**/vk_layer_settings.txt\n','')
            previous=self.retained_files(old,step,root)
            self.assertTrue(reports<=previous);self.assertFalse(required&previous)

    def test_source_uploader_retains_only_small_generated_pch_inputs_not_cache(self):
        with tempfile.TemporaryDirectory(prefix='renderer-pch-retention-') as directory:
            root=Path(directory);owner='build-r1/rhi-owned/source/build/lckernel/CMakeFiles/lckernel.dir/'
            required={owner+'cmake_pch.hxx',owner+'cmake_pch.hxx.cxx',owner+'cmake_pch.cxx',owner+'Release/cmake_pch.hxx'}
            excluded={owner+'cmake_pch.pch',owner+'cmake_pch.hxx.pch',owner+'cmake_pch.hxx.gch',
                owner+'first.cpp.o','outside/cmake_pch.hxx','outside/cmake_pch.hxx.cxx'}
            for name in required|excluded:
                path=root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(name)
            step='Retain selected source/provider/build/test/process evidence on failure too'
            selected=self.retained_files(self.text,step,root)
            self.assertTrue(required<=selected);self.assertFalse(excluded&selected)
            previous=re.sub(r'(?m)^            build-r1/\*\*/cmake_pch[^\n]*\n','',self.text)
            self.assertFalse(required&self.retained_files(previous,step,root))

    def test_spike_uploader_directory_globs_already_keep_validation_text_bytes(self):
        text=(inventory.REPO_ROOT/'.github/workflows/renderer.yml').read_text()
        with tempfile.TemporaryDirectory(prefix='renderer-spike-retention-') as directory:
            root=Path(directory);required=set()
            for build in ('build-rhi','build-rhi-noui'):
                for qpa in ('xcb','wayland'):
                    for name in ('checks.json','vk_layer_settings.txt'):
                        required.add(build+'/evidence-'+qpa+'/run-owned/'+name)
            for name in required|{'outside/vk_layer_settings.txt'}:
                path=root/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(name)
            selected=self.retained_files(text,'Retain R0 evidence, including failures',root)
            self.assertTrue(required<=selected);self.assertNotIn('outside/vk_layer_settings.txt',selected)

    def test_intel_metal_diagnostics_do_not_relax_device_classification(self):
        block=self.text.split('- name: Record native Intel Mac Metal adapter',1)[1].split('\n      - name:',1)[0]
        self.assertIn("matrix.platform == 'MAC-X64'",block)
        self.assertIn("matrix.tests == 'ON'",block)
        self.assertIn('system_profiler -json SPDisplaysDataType > build-r1/metal-adapter.json',block)
        source=(inventory.REPO_ROOT/'lcrenderer/rhi/tests/buildtest.cpp').read_text()
        self.assertIn('ASSERT_EQ(expectedDevice, info.deviceClass) << "Metal/Vulkan adapter: " << info.name;',source)

    def test_all_five_native_tuples_are_required_without_emulation_or_source_fallback(self):
        self.assertEqual({'LINUX-X64','LINUX-ARM64','MAC-X64','MAC-ARM64','WIN-X64'},set(re.findall(r'platform: ([A-Z0-9-]+)',self.text)))
        self.assertIn('fail-fast: false',self.text);self.assertIn('needs: [contracts, source]',self.text)
        self.assertIn("any(value['result']!='success' for value in results.values())",self.text)
        self.assertIn("'--build=never'",self.text);self.assertNotIn('--build=missing',self.text)
        self.assertIn('--force-bottle --ignore-dependencies',self.text)
        self.assertIn('if: always()',self.text);self.assertIn('include-hidden-files: true',self.text)
        self.assertIn('build-r1/**/cpu-headers/**',self.text)
        self.assertIn('build-r1/**/lcviewer_cpu_headercheck.dir/**',self.text)
        for output in ('lib*.so*','lib*.dylib','*.a','*.dll','*.lib'):
            self.assertIn('build-r1/**/'+output,self.text)
        self.assertIn('build-r1/**/*.qsb',self.text)
        self.assertIn('build-r1/**/python-runtime-inventory.txt',self.text)
        self.assertIn('workflow_call: null',self.text)
        self.assertIn("tests: ['ON', 'OFF']",self.text)
        self.assertIn('platform: [LINUX-X64, LINUX-ARM64, MAC-ARM64, MAC-X64, WIN-X64]',self.text)
        self.assertIn('R1_TESTS: ${{ matrix.tests }}',self.text)
        self.assertIn('name: renderer-r1-source-${{ matrix.contract_scope }}-${{ matrix.platform }}-tests-${{ matrix.tests }}',self.text)
        self.assertIn('contract_scope: [libraries-core, libraries-python, cli, cli-persistence, ui, both]',self.text)
        self.assertNotIn('        exclude:',self.text)
        platforms=[p.strip() for p in re.search(r'(?m)^        platform: \[([^\]]+)\]',self.text).group(1).split(',')]
        scopes=[p.strip() for p in re.search(r'(?m)^        contract_scope: \[([^\]]+)\]',self.text).group(1).split(',')]
        configurations={(p,t,s) for p in platforms for t in ('ON','OFF') for s in scopes}
        self.assertEqual(60,len(configurations));self.assertEqual(10,sum(s=='cli' for _,_,s in configurations))
        for group in ('libraries-core','libraries-python'):self.assertEqual(10,sum(s==group for _,_,s in configurations))
        self.assertIn("R1_SCOPE: ${{ startsWith(matrix.contract_scope, 'libraries-') && 'libraries' || matrix.contract_scope }}",self.text)
        self.assertIn("R1_LIBRARY_GROUP: ${{ matrix.contract_scope == 'libraries-core' && 'core' || matrix.contract_scope == 'libraries-python' && 'python' || 'all' }}",self.text)
        self.assertEqual(10,sum(s=='cli-persistence' for _,_,s in configurations))
        self.assertIn("if: runner.os == 'macOS' && (startsWith(matrix.contract_scope, 'libraries-')",self.text)
        self.assertIn('branches: [codex/r1-platform-ci]',self.text)
        for executable in ('luacmdinterface','lcunittest','lcrenderertest','lcbuildcontracttest'):
            self.assertIn('build-r1/**/bin/'+executable+'*',self.text)
        self.assertIn('uses: ./.github/workflows/renderer-source.yml',(inventory.REPO_ROOT/'.github/workflows/renderer.yml').read_text())
        self.assertNotRegex(self.text,r'(?m)^\s*(?:continue-on-error|secrets|pull_request_target):')
        reference=re.search(r'repository: KhronosGroup/Vulkan-Headers\n\s+ref: ([0-9a-f]{40})',self.text).group(1)
        # v1.4.341 is annotated: use its peeled commit, not tag object 0d3f509e.
        self.assertEqual('b5c8f996196ba4aa6d8f97e52b5d3b6e70f7e4e2',reference)
        self.assertIn('ref: '+reference,(inventory.REPO_ROOT/'.github/workflows/renderer.yml').read_text())

    def test_actual_shared_workflow_step_constructs_every_platform_command(self):
        code=self.text.split('- name: Run selected library or CLI source contracts',1)[1]
        code=textwrap.dedent(code.split('run: |\n',1)[1].split('\n        env:',1)[0])
        for platform in ('LINUX-X64','LINUX-ARM64','MAC-X64','MAC-ARM64','WIN-X64'):
            env=dict(R1_SCOPE='libraries',R1_PLATFORM=platform,R1_TESTS='ON',QT_ROOT_DIR='/provider',R1_LUA_INCLUDE='/lua/include',R1_LUA_LIBRARY='/lua/lib',
                     R1_VULKAN_ROOT='/vulkan',R1_TRIPLET='aarch64-linux-gnu' if platform=='LINUX-ARM64' else 'x86_64-linux-gnu',
                     VK_DRIVER_FILES='/vulkan/vk_swiftshader_icd.json',PATH=os.environ.get('PATH',''))
            with self.subTest(platform=platform),patch.dict(os.environ,env),patch.object(subprocess,'run') as run:
                exec(compile(code,'renderer-source.yml shared native step','exec'),{})
                commands=[c.args[0] for c in run.call_args_list]
                self.assertEqual(3 if platform.startswith('LINUX-') else 2,len(commands))
                rhi=commands[-1]
                expected='virtual' if platform=='MAC-X64' else 'hardware' if platform=='MAC-ARM64' else 'software'
                self.assertEqual(expected,rhi[rhi.index('--device')+1])
                self.assertEqual(platform=='MAC-X64','r1-source-virtual-metal/' in rhi[rhi.index('--configuration')+1])
                self.assertIn('--build-contracts',commands[0]);self.assertIn('cpu-source/'+platform+'/cpu/cpu/none/none',commands[0])
                for command in commands[1:]:
                    self.assertIn('--rhi-contracts',command);self.assertIn('--build-source',command);self.assertNotIn('--app',command)
                    self.assertTrue(any('/'+platform+'/rhi-offscreen/offscreen/' in v for v in command))
                    if platform.startswith('LINUX-'):
                        self.assertIn('--xvfb-run',command);self.assertIn('--vulkan-validation-library',command)
                if platform=='WIN-X64':
                    for variant in ('ordinary','empty'):self.assertTrue(any(v.startswith(variant+':CMAKE_TOOLCHAIN_FILE=') for v in commands[0]))
                    self.assertIn('--icd-manifest',commands[1]);self.assertIn('Visual Studio 17 2022',commands[1])
                    for profile in runner.LIBRARY_PROFILES:
                        self.assertTrue(any(v.startswith(profile+':CMAKE_TOOLCHAIN_FILE=') for v in commands[1]))
                self.assertIn('Python_EXECUTABLE='+sys.executable,commands[1])
                self.assertEqual(1,sum('--library-matrix' in c for c in commands))
                self.assertIn('--library-matrix',commands[1])
                if platform.startswith('LINUX-'):self.assertNotIn('--library-matrix',commands[2])

    def test_tests_off_step_is_compile_only_on_each_native_platform(self):
        code=self.text.split('- name: Run selected library or CLI source contracts',1)[1]
        code=textwrap.dedent(code.split('run: |\n',1)[1].split('\n        env:',1)[0])
        for platform in inventory.CASE_PLATFORMS:
            env=dict(R1_SCOPE='libraries',R1_PLATFORM=platform,R1_TESTS='OFF',QT_ROOT_DIR='/provider',R1_LUA_INCLUDE='/lua/include',R1_LUA_LIBRARY='/lua/lib',
                     R1_TRIPLET='aarch64-linux-gnu' if platform=='LINUX-ARM64' else 'x86_64-linux-gnu')
            with self.subTest(platform=platform),patch.dict(os.environ,env),patch.object(subprocess,'run') as run:
                with self.assertRaises(SystemExit) as stopped:exec(compile(code,'tests-OFF shared native step','exec'),{})
                self.assertEqual(0,stopped.exception.code);self.assertEqual(1,run.call_count)
                command=run.call_args.args[0]
                self.assertIn('--production-builds',command);self.assertNotIn('--rhi-contracts',command)
                self.assertNotIn('--build-contracts',command);self.assertNotIn('--xvfb-run',command)
                self.assertEqual(platform,command[command.index('--platform')+1])
                if platform=='WIN-X64':
                    overrides=[command[i+1] for i,v in enumerate(command) if v=='--case-cmake-option']
                    self.assertEqual(8,len(overrides))
                    for value in overrides:
                        name=value.partition(':')[0];case='empty' if name=='cpu' else name+'-no-tests'
                        self.assertIn(str(Path('build-r1/conan',case,'conan_toolchain.cmake').resolve()),value)
        self.assertIn('test_dependencies=(libgtest-dev libgmock-dev)',self.text)
        self.assertIn('if test "$R1_TESTS" = ON; then core+=(googletest); fi',self.text)

    def test_each_required_library_group_is_forwarded_without_weakening_qpa_checks(self):
        code=self.text.split('- name: Run selected library or CLI source contracts',1)[1]
        code=textwrap.dedent(code.split('run: |\n',1)[1].split('\n        env:',1)[0])
        for group in ('core','python'):
            for tests in ('ON','OFF'):
                env=dict(R1_SCOPE='libraries',R1_LIBRARY_GROUP=group,R1_PLATFORM='LINUX-ARM64',R1_TESTS=tests,
                    QT_ROOT_DIR='/provider',R1_TRIPLET='aarch64-linux-gnu',VK_DRIVER_FILES='/vulkan/icd.json')
                with self.subTest(group=group,tests=tests),patch.dict(os.environ,env),patch.object(subprocess,'run') as run:
                    if tests=='OFF':
                        with self.assertRaises(SystemExit):exec(compile(code,'grouped libraries','exec'),{})
                    else:exec(compile(code,'grouped libraries','exec'),{})
                    commands=[c.args[0] for c in run.call_args_list]
                    grouped=[c for c in commands if '--library-group' in c]
                    self.assertEqual(1,len(grouped));self.assertEqual(group,grouped[0][grouped[0].index('--library-group')+1])
                    if tests=='ON':
                        self.assertEqual(3,len(commands));self.assertIn('--library-matrix',commands[1])
                        self.assertIn('--weston',commands[2]);self.assertNotIn('--library-group',commands[2])

    def test_actual_cli_ci_scope_has_eight_explicit_binary_only_off_path_commands(self):
        code=self.text.split('- name: Run selected library or CLI source contracts',1)[1]
        code=textwrap.dedent(code.split('run: |\n',1)[1].split('\n        env:',1)[0])
        for platform in inventory.CASE_PLATFORMS:
            for tests in ('ON','OFF'):
                profile='dual-python' if tests=='ON' else 'legacy'
                env=dict(R1_SCOPE='cli',R1_PLATFORM=platform,R1_TESTS=tests,R1_LUA_INCLUDE='/lua/include',
                         R1_LUA_LIBRARY='/lua/lib',R1_BREW_PREFIX='/brew',R1_HOST_PROFILE=profile,QT_ROOT_DIR='/qt',PATH=os.environ.get('PATH',''))
                with self.subTest(platform=platform,tests=tests),patch.dict(os.environ,env,clear=True),patch.object(subprocess,'run') as run:
                    with self.assertRaises(SystemExit) as ended:exec(compile(code,'CLI source scope','exec'),{})
                    self.assertEqual(0,ended.exception.code);self.assertEqual(1,run.call_count)
                    command=run.call_args.args[0]
                    for flag in ('--host-builds','--host-no-persistence','--host-install'):self.assertIn(flag,command)
                    self.assertEqual('cli',command[command.index('--host-kind')+1])
                    self.assertEqual(tests,command[command.index('--host-tests')+1])
                    self.assertEqual(profile,command[command.index('--host-profile')+1])
                    self.assertNotIn('--rhi-contracts',command);self.assertNotIn('--production-builds',command)
                    self.assertNotIn('--provider-prefix',command);self.assertNotIn('--xvfb-run',command)
                    self.assertEqual('Visual Studio 17 2022' if platform=='WIN-X64' else 'Ninja',
                        command[command.index('--generator')+1])
                    if platform.startswith('MAC-'):self.assertIn('CMAKE_PREFIX_PATH='+('/qt;' if tests=='ON' else '')+'/brew',command)

    def test_linux_dependency_arrays_select_cli_without_qt_and_only_requested_gtest(self):
        text=self.text.split('- name: Install Linux binary dependencies',1)[1]
        text=textwrap.dedent(text.split('run: |\n',1)[1].split('\n      - name:',1)[0])
        arrays='test_dependencies=()'+text.split('test_dependencies=()',1)[1].split('sudo apt-get install',1)[0]
        self.assertNotRegex(arrays,r'\b(?:sudo|apt-get|brew|curl)\s')
        shell=shutil.which('bash')
        if not shell:self.skipTest('Linux binary-provider arrays require Bash, not used by Windows')
        for scope in ('libraries','cli','cli-persistence','ui','both'):
            for tests in ('ON','OFF'):
                result=subprocess.run([shell,'-c',arrays+'\nprintf "%s\\n" "${test_dependencies[@]}" "${scoped_dependencies[@]}"'],
                    env=dict(os.environ,R1_SCOPE=scope,R1_TESTS=tests),capture_output=True,text=True,timeout=10)
                self.assertEqual(0,result.returncode,result.stderr);packages=result.stdout.splitlines()
                self.assertEqual(tests=='ON','libgtest-dev' in packages)
                self.assertEqual(scope in ('cli','cli-persistence','both'),'libcurl4-gnutls-dev' in packages)
                self.assertEqual(scope in ('cli','cli-persistence','both'),'libglfw3-dev' in packages)
                self.assertEqual(scope in ('libraries','ui','both') or tests=='ON',any(p.startswith('qt6-base-dev=') for p in packages))
                self.assertEqual(scope in ('ui','both'),'qt6-svg-dev=6.10.2-2' in packages)

    def test_linux_x64_mirror_replacement_preserves_other_providers_and_priorities(self):
        sed=shutil.which('sed')
        if not sed:self.skipTest('Linux mirror rewrite requires sed, not used by Windows')
        block=self.text.split('- name: Install Linux binary dependencies',1)[1].split('sudo apt-get update',1)[0]
        self.assertIn('if test "$R1_PLATFORM" = LINUX-X64; then',block)
        expression=re.search(r"sudo sed -i '([^']+)' /etc/apt/apt-mirrors.txt",block).group(1)
        source=('http://azure.archive.ubuntu.com/ubuntu priority:1\n'
            'https://archive.ubuntu.com/ubuntu priority:2\n'
            'http://ports.ubuntu.com/ubuntu-ports priority:1\n'
            'https://packages.microsoft.com/ubuntu/26.04/prod priority:1\n'
            'http://azureXarchiveXubuntuXcom/ubuntu priority:3\n')
        result=subprocess.run([sed,expression],input=source,capture_output=True,text=True,timeout=10)
        self.assertEqual(0,result.returncode,result.stderr)
        self.assertEqual(source.replace('http://azure.archive.ubuntu.com/ubuntu','https://archive.ubuntu.com/ubuntu'),result.stdout)
        self.assertNotRegex(block,r'(?i)allow-unauthenticated|trusted=yes|check-valid-until')

    def test_persistence_hosts_build_the_vendored_libdxfrw_without_an_sdk_step(self):
        # Persistence hosts compile third_party/libdxfrw in-tree: no SDK is built,
        # passed, or found, on any platform or persistence scope.
        code=self.text.split('- name: Run selected library or CLI source contracts',1)[1]
        code=textwrap.dedent(code.split('run: |\n',1)[1].split('\n        env:',1)[0])
        for platform in inventory.CASE_PLATFORMS:
          for scope in ('cli-persistence','ui','both'):
            for tests in ('ON','OFF'):
                env=dict(R1_SCOPE=scope,R1_PLATFORM=platform,R1_TESTS=tests,
                    R1_HOST_PROFILE='dual-python' if tests=='ON' else 'legacy',QT_ROOT_DIR='/qt',
                    R1_BREW_PREFIX='/brew',R1_LUA_INCLUDE='/lua/include',R1_LUA_LIBRARY='/lua/lib')
                with self.subTest(platform=platform,scope=scope,tests=tests),patch.dict(os.environ,env), \
                        patch.object(subprocess,'run') as run:
                    with self.assertRaises(SystemExit) as stopped:exec(compile(code,'source persistence CLI step','exec'),{})
                    self.assertEqual(0,stopped.exception.code);self.assertEqual(1,run.call_count)
                    command=run.call_args.args[0]
                    self.assertNotIn('--host-no-persistence',command);self.assertEqual(scope=='cli-persistence','--host-install' in command)
                    self.assertEqual('cli' if scope=='cli-persistence' else scope,command[command.index('--host-kind')+1])
                    self.assertFalse([a for a in command if re.match(r'(?:LIBDXFRW_|libdxfrw_DIR)',a)])
        self.assertNotIn('- name: Build pinned source-submodule persistence SDK',self.text)
        self.assertNotIn('build_libdxfrw.py',self.text)

    def test_windows_setup_validates_metadata_before_install_and_locks_the_same_route(self):
        code=self.text.split('- name: Restore Windows core binaries and exact per-case toolchains',1)[1]
        code=textwrap.dedent(code.split('run: |\n',1)[1].split('\n      - name:',1)[0])
        with tempfile.TemporaryDirectory(prefix='conan-workflow-fixture-') as tmp, \
             patch.dict(os.environ,R1_SCOPE='libraries',R1_TESTS='ON',RUNNER_TEMP=tmp,GITHUB_ENV=str(Path(tmp)/'env'),GITHUB_PATH=str(Path(tmp)/'path')), \
             patch('validate_renderer_conan_graph.windows_runtime_directories',return_value=[str(Path(tmp)/'LuaBin')]) as runtime, \
             patch.object(subprocess,'run') as run:
            exec(compile(code,'renderer-source.yml Windows Conan step','exec'),{})
            self.assertEqual(str(Path(tmp)/'LuaBin')+'\n',(Path(tmp)/'path').read_text())
            self.assertEqual(9,runtime.call_count)
        commands=[c.args[0] for c in run.call_args_list]
        graphs=[i for i,c in enumerate(commands) if c[:3]==['conan','graph','info']]
        self.assertEqual(9,len(graphs))
        for i in graphs:
            before=commands[i];name=before[before.index('--out-file')+1]
            for profile in ('ordinary','empty','rhi','legacy','dual','cpu-python','legacy-python','dual-python','rhi-python'):
                if name.endswith('conan-'+profile+'-preflight.json'):
                    self.assertIn('&:with_legacy_opengl_renderer='+str(profile.removesuffix('-python') in ('legacy','dual')),before)
                    self.assertIn('&:with_pythonscript='+str(profile.endswith('-python')),before)
                    self.assertIn('&:with_renderer_tests='+str(profile=='rhi'),before)
        for i in graphs:
            before,check,install,after=commands[i:i+4]
            self.assertIn('--build=never',before);self.assertIn('--build=never',install)
            for command in (before,install):
                self.assertIn(['-c','tools.cmake.cmaketoolchain:user_presets='],[command[j:j+2] for j in range(len(command)-1)])
            self.assertEqual(['conan','install'],install[:2])
            lock=before[before.index('--lockfile-out')+1]
            self.assertEqual(lock,install[install.index('--lockfile')+1])
            self.assertIn('scripts/validate_renderer_conan_graph.py',check)
            self.assertIn('--expected-route',after)
            self.assertEqual(check[check.index('--report')+1],after[after.index('--expected-route')+1])
        self.assertIn('build-r1/**/*.lock',self.text)

    def test_windows_tests_off_routes_never_request_gtest_or_renderer_tests(self):
        code=self.text.split('- name: Restore Windows core binaries and exact per-case toolchains',1)[1]
        code=textwrap.dedent(code.split('run: |\n',1)[1].split('\n      - name:',1)[0])
        with tempfile.TemporaryDirectory(prefix='conan-tests-off-fixture-') as tmp, \
             patch.dict(os.environ,R1_SCOPE='libraries',R1_TESTS='OFF',RUNNER_TEMP=tmp,GITHUB_ENV=str(Path(tmp)/'env'),GITHUB_PATH=str(Path(tmp)/'path')), \
             patch('validate_renderer_conan_graph.windows_runtime_directories',return_value=[str(Path(tmp)/'LuaBin')]) as runtime, \
             patch.object(subprocess,'run') as run:
            exec(compile(code,'tests-OFF Conan step','exec'),{})
            self.assertEqual(str(Path(tmp)/'LuaBin')+'\n',(Path(tmp)/'path').read_text())
            self.assertEqual(8,runtime.call_count)
        commands=[c.args[0] for c in run.call_args_list];graphs=[c for c in commands if c[:3]==['conan','graph','info']]
        self.assertEqual(8,len(graphs))
        for command in graphs:
            self.assertIn('&:with_unittests=False',command);self.assertIn('&:with_renderer_tests=False',command)
            self.assertIn('--build=never',command)
        self.assertEqual(8,sum(c[:2]==['conan','install'] for c in commands))

    def test_windows_profiles_cannot_publish_different_lua_runtimes(self):
        code=self.text.split('- name: Restore Windows core binaries and exact per-case toolchains',1)[1]
        code=textwrap.dedent(code.split('run: |\n',1)[1].split('\n      - name:',1)[0])
        with tempfile.TemporaryDirectory(prefix='conan-runtime-workflow-') as tmp, \
             patch.dict(os.environ,R1_SCOPE='libraries',R1_TESTS='OFF',RUNNER_TEMP=tmp,GITHUB_ENV=str(Path(tmp)/'env'),GITHUB_PATH=str(Path(tmp)/'path')), \
             patch('validate_renderer_conan_graph.windows_runtime_directories',side_effect=[[str(Path(tmp)/('LuaBin'+str(i)))] for i in range(8)]), \
             patch.object(subprocess,'run'):
            with self.assertRaisesRegex(ValueError,'one installed Lua runtime'):exec(compile(code,'inconsistent Lua runtime step','exec'),{})
            self.assertFalse((Path(tmp)/'path').exists());self.assertFalse((Path(tmp)/'env').exists())

    def test_shared_sdk_probe_checks_link_stubs_not_just_a_framework_directory(self):
        cmake=shutil.which('cmake');self.assertTrue(cmake)
        helper=inventory.REPO_ROOT/'lcrenderer/rhi/spike/cmake/LegacyQtMacSdk.cmake'
        with tempfile.TemporaryDirectory(prefix='rhi-sdk-fixture-') as tmp:
            root=Path(tmp);sdk=root/'sdk';framework=sdk/'System/Library/Frameworks/AGL.framework';framework.mkdir(parents=True)
            for present in (False,True):
                if present:(framework/'AGL.tbd').write_text('synthetic-stub-not-a-linkable-library')
                driver=root/('present.cmake' if present else 'absent.cmake')
                driver.write_text('set(APPLE TRUE)\nset(CMAKE_OSX_SYSROOT "'+sdk.as_posix()+'")\ninclude("'+helper.as_posix()+'")\n'
                    'lc_spike_sdk_has_agl("6.8.3" actual)\nif(NOT actual STREQUAL "'+str(present).lower()+'")\n'
                    'message(FATAL_ERROR "Wrong selected SDK link-stub availability")\nendif()\n')
                result=subprocess.run([cmake,'-P',str(driver)],capture_output=True,timeout=10)
                self.assertEqual(0,result.returncode,result.stdout+result.stderr)

    def test_linux_uses_its_paired_system_embed_sdk_not_an_unavailable_action_archive(self):
        blocks=self.text.split('matrix:\n',1)[1].split('    runs-on:',1)[0]
        for block in blocks.split('- runner:')[1:]:
            self.assertIn('python: /usr/bin/python3' if 'platform: LINUX-' in block else 'python: python',block)
        setup=self.text.split('- uses: actions/setup-python@',1)[1].split('      - name:',1)[0]
        self.assertIn("if: runner.os != 'Linux'",setup);self.assertIn("python-version: '3.12.10'",setup)
        self.assertIn('python3-dev',self.text)
        self.assertNotIn('shell: python -B {0}',self.text)
        self.assertIn('defaults:\n      run:\n        shell: ${{ matrix.python }} -B {0}',self.text)
        self.assertEqual(1,self.text.count('shell: ${{ matrix.python }} -B {0}'))


if __name__=='__main__':unittest.main()
