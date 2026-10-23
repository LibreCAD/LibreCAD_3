"""CPU-only Weston lifecycle/identity/safety negatives; not Wayland render proof."""

import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import Mock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import renderer_wayland as wayland


class WestonSessionTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        prefix = self.root / "provider"
        self.binary = prefix / "bin/weston"
        self.paths = [self.binary, prefix / "lib/test-linux-gnu/libweston-14/headless-backend.so",
                      prefix / "lib/test-linux-gnu/weston/desktop-shell.so",
                      prefix / "libexec/weston-desktop-shell", prefix / "libexec/weston-keyboard"]
        for path in self.paths:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"test provider bytes")
        self.report = self.root / "evidence"
        self.report.mkdir()
        self.platform = patch.object(wayland.sys, "platform", "linux")
        self.platform.start()
        self.addCleanup(self.platform.stop)
        self.triplet = patch.object(wayland.sysconfig, "get_config_var", return_value="test-linux-gnu")
        self.triplet.start()
        self.addCleanup(self.triplet.stop)

    def session(self, **kwargs):
        return wayland.WestonSession(self.report, self.binary, **kwargs)

    def test_wrong_os_and_missing_input_display_are_rejected(self):
        with patch.object(wayland.sys, "platform", "darwin"), self.assertRaisesRegex(ValueError, "requires Linux"):
            self.session()
        with patch.dict(os.environ, {"DISPLAY": ""}), self.assertRaisesRegex(ValueError, "parent X11"):
            self.session(backend="x11")

    def test_input_cannot_target_remote_or_nonvirtual_desktop(self):
        for display in ("host:1", "", "wayland-0"):
            with self.subTest(display=display), self.assertRaisesRegex(ValueError, "isolated Xvfb"):
                wayland.WestonSession.require_xvfb(display)
        with patch.object(Path, "read_text", return_value="42"), \
             patch.object(Path, "read_bytes", return_value=b"/usr/bin/Xorg\0:0\0"), \
             self.assertRaisesRegex(ValueError, "non-Xvfb"):
            wayland.WestonSession.require_xvfb(":0")

    def test_native_input_acknowledgment_is_explicit_and_strict(self):
        path = self.report / "wayland-input.json"
        marker = {"key": "F12", "qpa": "wayland", "source": "spontaneous-key-event", "owner_generation": 1}
        path.write_text(json.dumps(marker))
        self.assertEqual(marker, wayland.validate_input_marker(path))
        session = self.session()
        self.assertTrue(session.observe_input())
        for key, value in (("key", "F24"), ("qpa", "xcb"), ("source", "synthetic-Qt-event"),
                           ("owner_generation", True), ("owner_generation", 2)):
            path.write_text(json.dumps({**marker, key: value}))
            with self.subTest(key=key), self.assertRaises(ValueError):
                wayland.validate_input_marker(path)

    def test_unqualified_version_does_not_start_a_compositor(self):
        with patch.object(wayland.subprocess, "run", return_value=subprocess.CompletedProcess([], 0, b"weston 99.0.0", b"")), \
             patch.object(wayland.subprocess, "Popen") as create, self.assertRaisesRegex(ValueError, "Unqualified Weston"):
            with self.session():
                self.fail("unqualified provider entered session")
        create.assert_not_called()

    def test_failed_start_keeps_logs_and_never_launches_an_app(self):
        proc = Mock(pid=12345, returncode=1)
        proc.poll.return_value = 1
        with patch.object(wayland.subprocess, "run", return_value=subprocess.CompletedProcess([], 0, b"weston 14.0.2", b"")), \
             patch.object(wayland.subprocess, "Popen", return_value=proc), \
             patch.object(wayland.os, "killpg") as kill, self.assertRaisesRegex(ValueError, "before.*socket"):
            with self.session():
                self.fail("failed compositor entered session")
        kill.assert_not_called()
        self.assertTrue((self.report / "weston.stderr.log").exists())

    def test_deep_evidence_paths_do_not_become_unix_socket_paths(self):
        self.report = self.report / ("long-evidence-" * 12)
        self.report.mkdir()
        factory = tempfile.TemporaryDirectory
        proc = Mock(pid=12345, returncode=1)
        proc.poll.return_value = 1
        with patch.object(wayland.subprocess, "run", return_value=subprocess.CompletedProcess([], 0, b"weston 14.0.2", b"")), \
             patch.object(wayland.subprocess, "Popen", return_value=proc), \
             patch.object(wayland.tempfile, "TemporaryDirectory", wraps=factory) as create, \
             self.assertRaisesRegex(ValueError, "before.*socket"):
            with self.session():
                self.fail("fake compositor entered session")
        self.assertEqual("/tmp", create.call_args.kwargs["dir"])

    def configured(self):
        session = self.session()
        session.proc = Mock(pid=12345, returncode=0)
        session.proc.poll.return_value = None
        session.command = [str(self.binary), "--backend=headless", "--renderer=pixman"]
        session.client_env = {"QT_QPA_PLATFORM": "wayland"}
        (self.report / "weston.log").write_text("Using Pixman renderer\n" + str(session.modules["headless-backend.so"]))
        return session

    def test_app_exit_status_is_not_replaced_by_compositor_success(self):
        session = self.configured()
        with patch.object(wayland.subprocess, "run", return_value=subprocess.CompletedProcess([], 7, b"", b"app failed")):
            result = session.execute(["app"], 1)
        self.assertEqual(7, result.returncode)
        self.assertEqual(b"app failed", result.stderr)

    def test_success_requires_live_server_correct_renderer_and_unchanged_inputs(self):
        for fault in ("none", "server-exited", "wrong-renderer", "changed-input"):
            session = self.configured()
            if fault == "server-exited":
                session.proc.poll.return_value = 0
            elif fault == "wrong-renderer":
                (self.report / "weston.log").write_text("Using noop renderer")
            elif fault == "changed-input":
                self.binary.write_bytes(b"changed provider")
            with patch.object(wayland.os, "killpg"):
                if fault == "none":
                    session.__exit__(None, None, None)
                    record = json.loads((self.report / "weston.json").read_text())
                    self.assertTrue(record["live_until_app_exit"])
                    self.assertTrue(record["app_display_unset"])
                else:
                    with self.subTest(fault=fault), self.assertRaises(ValueError):
                        session.__exit__(None, None, None)
            self.binary.write_bytes(b"test provider bytes")


if __name__ == "__main__":
    unittest.main()
