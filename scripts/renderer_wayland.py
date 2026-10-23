"""Private R0 Weston session; the app and compositor have separate exit status."""

import json
import os
import re
import signal
import stat
import subprocess
import sys
import sysconfig
import tempfile
import time
from pathlib import Path

from validate_renderer_inventory import read_json, sha256


def validate_input_marker(path):
    marker = read_json(path)
    if (not isinstance(marker, dict) or marker.get("key") != "F12" or marker.get("qpa") != "wayland" or
            marker.get("source") != "spontaneous-key-event" or type(marker.get("owner_generation")) is not int or
            marker["owner_generation"] != 1):
        raise ValueError("Invalid native compositor input acknowledgment")
    return marker


class WestonSession:
    files = {"weston.log", "weston.stdout.log", "weston.stderr.log", "weston.json"}

    def __init__(self, directory, executable, prefix=None, backend="headless", input_tool=None):
        if sys.platform != "linux" or backend not in ("headless", "x11"):
            raise ValueError("Weston proof requires Linux and an explicit headless or x11 backend")
        self.directory, self.backend = directory.resolve(), backend
        self.executable = executable.resolve(strict=True)
        prefix = (prefix or self.executable.parent.parent).resolve(strict=True)
        lib = prefix / "lib" / sysconfig.get_config_var("MULTIARCH")
        self.modules = {backend + "-backend.so": lib / "libweston-14" / (backend + "-backend.so"),
                        "desktop-shell.so": lib / "weston/desktop-shell.so",
                        "weston-desktop-shell": prefix / "libexec/weston-desktop-shell",
                        "weston-keyboard": prefix / "libexec/weston-keyboard"}
        self.env = os.environ.copy()
        self.parent_display = self.env.get("DISPLAY", "")
        self.input_tool = input_tool.resolve(strict=True) if input_tool else None
        if backend == "x11" and not self.parent_display:
            raise ValueError("Nested Weston requires its declared parent X11 display")
        if self.input_tool:
            self.require_xvfb(self.parent_display)
            if backend != "x11":
                raise ValueError("Native popup input requires the nested x11 compositor seat")
        self.inputs = [self.executable, *self.modules.values()] + ([self.input_tool] if self.input_tool else [])
        self.digests = self.fingerprints()
        self.env["LD_LIBRARY_PATH"] = os.pathsep.join([str(lib), str(lib / "weston"), self.env.get("LD_LIBRARY_PATH", "")])
        self.env["WESTON_MODULE_MAP"] = ";".join(name + "=" + str(path) for name, path in self.modules.items())
        self.env["WESTON_DATA_DIR"] = str(prefix / "share/weston")
        self.proc = self.runtime = None
        self.streams = []
        self.record = {}
        self.input_count = 0
        self.input_window = None
        self.input_observed = False
        self.files = set(self.files)
        if self.input_tool:
            self.files.add("wayland-input.json")

    @staticmethod
    def require_xvfb(display):
        match = re.fullmatch(r":(\d+)(?:\.\d+)?", display)
        if not match:
            raise ValueError("Input seeding requires a local isolated Xvfb display")
        pid = int(Path("/tmp/.X" + match[1] + "-lock").read_text().strip())
        command = Path("/proc/{}/cmdline".format(pid)).read_bytes().split(b"\0")
        if Path(os.fsdecode(command[0])).name != "Xvfb" or (":" + match[1]).encode() not in command:
            raise ValueError("Input seeding refuses a non-Xvfb display")

    def fingerprints(self):
        return {str(path): sha256(path.read_bytes()) for path in self.inputs}

    def __enter__(self):
        try:
            version = subprocess.run([str(self.executable), "--version"], env=self.env,
                                     capture_output=True, timeout=5, check=True).stdout.decode().strip()
            if version != "weston 14.0.2":
                raise ValueError("Unqualified Weston provider: " + version)
            # Unix-domain socket paths include the directory and must fit in
            # sockaddr_un (108 bytes on Linux), even with deeply nested reports.
            self.runtime = tempfile.TemporaryDirectory(prefix="lc-wayland-", dir="/tmp")
            os.chmod(self.runtime.name, 0o700)
            self.env.update(XDG_RUNTIME_DIR=self.runtime.name, WAYLAND_DISPLAY="librecad-r0")
            self.env.pop("WAYLAND_SOCKET", None)
            self.command = [str(self.executable), "--backend=" + self.backend, "--renderer=pixman", "--shell=desktop",
                "--socket=librecad-r0", "--idle-time=0", "--width=2048", "--height=1536", "--no-config",
                "--log=" + str(self.directory / "weston.log")]
            self.streams = [(self.directory / name).open("wb") for name in ("weston.stdout.log", "weston.stderr.log")]
            self.proc = subprocess.Popen(self.command, env=self.env, stdin=subprocess.DEVNULL,
                stdout=self.streams[0], stderr=self.streams[1], start_new_session=True)
            deadline = time.monotonic() + 10
            socket = Path(self.runtime.name) / "librecad-r0"
            while time.monotonic() < deadline:
                if self.proc.poll() is not None:
                    raise ValueError("Weston exited before its Wayland socket was ready")
                if socket.exists() and stat.S_ISSOCK(socket.stat().st_mode):
                    self.client_env = self.env.copy()
                    self.client_env.pop("DISPLAY", None) # Never run the CAD app through XWayland/XCB.
                    self.client_env["QT_QPA_PLATFORM"] = "wayland"
                    return self
                time.sleep(0.02)
            raise ValueError("Weston socket readiness timed out")
        except BaseException:
            self.close()
            raise

    def execute(self, command, timeout):
        if not self.input_tool:
            return subprocess.run(command, env=self.client_env, capture_output=True, timeout=timeout)
        proc = subprocess.Popen(command, env=self.client_env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        deadline = time.monotonic() + timeout
        try:
            while time.monotonic() < deadline:
                try:
                    out, err = proc.communicate(timeout=min(0.1, max(0.001, deadline - time.monotonic())))
                    self.observe_input()
                    return subprocess.CompletedProcess(command, proc.returncode, out, err)
                except subprocess.TimeoutExpired:
                    if self.proc.poll() is not None:
                        raise ValueError("Weston stopped while the app was running")
                    if self.observe_input():
                        continue # Do not send more keys into an opened popup or replacement view.
                    log = (self.directory / "weston.log").read_text(errors="replace")
                    window = re.search(r"x11 output \d+x\d+, window id (\d+)", log)
                    if not window:
                        raise ValueError("No observed Weston X11 window for targeted input")
                    # Only this compositor's window on a verified Xvfb server.
                    # XSendEvent (--window) is not native seat input. Focus only
                    # our compositor on the verified virtual server, then XTest.
                    subprocess.run([str(self.input_tool), "windowfocus", "--sync", window[1], "key", "F12"],
                                   env=self.env, capture_output=True, timeout=2, check=True)
                    self.input_window = window[1]
                    self.input_count += 1
            raise subprocess.TimeoutExpired(command, timeout)
        finally:
            if proc.poll() is None:
                proc.kill()
                out, err = proc.communicate(timeout=5)
                # Preserve partial app diagnostics on errors outside communicate().
                (self.directory / "stdout.log").write_bytes(out)
                (self.directory / "stderr.log").write_bytes(err)

    def observe_input(self):
        path = self.directory / "wayland-input.json"
        if path.exists():
            validate_input_marker(path)
            self.input_observed = True
        return self.input_observed

    def close(self):
        if self.proc and self.proc.poll() is None:
            try:
                os.killpg(self.proc.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
            try:
                self.proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                os.killpg(self.proc.pid, signal.SIGKILL)
                self.proc.wait(timeout=5)
        for stream in self.streams:
            stream.close()
        if self.runtime:
            self.runtime.cleanup()

    def __exit__(self, kind, value, traceback):
        live = self.proc.poll() is None
        self.close()
        self.record = {"version": "14.0.2", "backend": self.backend, "renderer": "pixman", "command": self.command,
            "inputs_sha256": self.digests, "socket_ready": True, "live_until_app_exit": live,
            "private_runtime": True, "app_display_unset": "DISPLAY" not in self.client_env,
            "parent_x11_display": self.parent_display if self.backend == "x11" else None,
            "targeted_f12_events": self.input_count, "shutdown_returncode": self.proc.returncode}
        self.record.update(input_x11_window=self.input_window,
                           input_method="XTest-F12-on-owned-Xvfb-compositor" if self.input_tool else None,
                           native_input_acknowledged=self.input_observed)
        (self.directory / "weston.json").write_text(json.dumps(self.record, indent=2) + "\n", encoding="utf-8")
        if kind is None:
            log = (self.directory / "weston.log").read_text(errors="replace")
            if not live or self.proc.returncode not in (0, -signal.SIGTERM):
                raise ValueError("Weston stopped unexpectedly or failed cleanup")
            if "Using Pixman renderer" not in log or str(self.modules[self.backend + "-backend.so"]) not in log:
                raise ValueError("Weston did not initialize the requested backend/renderer")
            if self.fingerprints() != self.digests:
                raise ValueError("Compositor/input binary changed during execution")
            if self.input_tool and (self.input_count < 1 or not self.input_observed):
                raise ValueError("Requested compositor input was not executed/acknowledged")
