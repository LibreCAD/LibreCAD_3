"""Static safeguards for the macOS runtime app-bundle payload."""

import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class MacOSBundleContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.script = (
            REPOSITORY_ROOT / "scripts/macos-install/createBundle.sh"
        ).read_text(encoding="utf-8")

    def test_bundle_stages_only_runtime_libraries_from_install_prefix(self):
        self.assertIn(
            'runtime_libraries=("$prefix"/lib/*.dylib "$prefix"/lib/*.framework)',
            self.script,
        )
        self.assertIn('ditto "$runtime_library" "$frameworks/"', self.script)
        self.assertNotIn('ditto "$prefix/lib/." "$frameworks"', self.script)
        self.assertNotIn('"$prefix"/lib/*.a', self.script)
        self.assertIn("No runtime dylibs or frameworks found", self.script)


if __name__ == "__main__":
    unittest.main()
