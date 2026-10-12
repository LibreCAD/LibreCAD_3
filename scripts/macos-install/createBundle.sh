#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <installed-prefix> <output.app>" >&2
    exit 2
fi

prefix=$(cd "$1" && pwd -P)
bundle=$2
if [[ "$bundle" != /* ]]; then
    bundle="$(pwd)/$bundle"
fi
if [[ -e "$bundle" ]]; then
    echo "Refusing to overwrite existing bundle: $bundle" >&2
    exit 1
fi

for required in \
    "$prefix/bin/librecad" \
    "$prefix/lib/libpython3"*.dylib \
    "$prefix/share/librecad/python/lib/python"*/os.py \
    "$prefix/share/librecad/lcUIPy/__init__.py"; do
    if [[ ! -e "$required" ]]; then
        echo "Required installed file is missing: $required" >&2
        exit 1
    fi
done

script_dir=$(cd "$(dirname "$0")" && pwd -P)
macdeployqt_bin=${MACDEPLOYQT:-$(command -v macdeployqt || true)}
qtpaths_bin=${QTPATHS:-$(command -v qtpaths6 || command -v qtpaths || true)}
otool_bin=${OTOOL:-$(command -v otool || true)}
install_name_tool_bin=${INSTALL_NAME_TOOL:-$(command -v install_name_tool || true)}
codesign_bin=${CODESIGN:-$(command -v codesign || true)}
python_bin=${PYTHON_EXECUTABLE:-$(command -v python3 || true)}
for tool in macdeployqt_bin qtpaths_bin otool_bin install_name_tool_bin codesign_bin python_bin; do
    if [[ -z "${!tool}" || ! -x "${!tool}" ]]; then
        echo "Required macOS packaging tool is unavailable: $tool" >&2
        exit 1
    fi
done

install_origins="$prefix/share/librecad/licenses/providers/native-origins.json"
if [[ -e "$install_origins" || -L "$install_origins" ]]; then
    # Validate this stage before copying it. Its prefix-relative paths/final
    # hashes cannot describe the later Qt-deployed and re-signed .app.
    "$python_bin" -B -c '
import sys
from pathlib import Path
root = Path(sys.argv[1])
sys.path.insert(0, str(root / "share/librecad/lcUIPy"))
from _nativeorigins import audit_native_origins
print("Verified install-stage native origins:", audit_native_origins(root, sys.argv[2]))
' "$prefix" "$install_origins"
fi

contents="$bundle/Contents"
macos="$contents/MacOS"
frameworks="$contents/Frameworks"
resources="$contents/Resources"
mkdir -p "$macos" "$frameworks" "$resources"
ditto "$script_dir/Info.plist" "$contents/Info.plist"
ditto "$prefix/bin/librecad" "$macos/librecad"
# Stage runtime objects only. Static archives are link-time inputs and cannot
# be loaded or deep-signed as app-bundle components.
for runtime_library in "$prefix"/lib/*.dylib "$prefix"/lib/*.so; do
    if [[ -e "$runtime_library" || -L "$runtime_library" ]]; then
        ditto "$runtime_library" "$frameworks/$(basename "$runtime_library")"
    fi
done
for runtime_bundle in "$prefix"/lib/*.framework "$prefix"/lib/*.bundle; do
    if [[ -d "$runtime_bundle" ]]; then
        ditto "$runtime_bundle" "$frameworks/$(basename "$runtime_bundle")"
    fi
done
ditto "$prefix/share/librecad" "$resources/librecad"
if [[ -e "$install_origins" ]]; then
    retained_origins="$resources/librecad/licenses/providers/native-origins-install-prefix.json"
    if [[ -e "$retained_origins" || -L "$retained_origins" ]]; then
        echo "Refusing to replace retained install-stage native origins: $retained_origins" >&2
        exit 1
    fi
    mv "$resources/librecad/licenses/providers/native-origins.json" "$retained_origins"
    echo "Retained install-prefix origins; final app native-origin qualification remains separate"
fi

# Python installs can include optional development-library links in their
# sysconfig directory even when those libraries are not installed. Omit only
# the known missing archive/shared-library links; any other dangling symlink
# is a packaging error and must remain visible.
python_root="$resources/librecad/python"
while IFS= read -r -d '' python_link; do
    if [[ -e "$python_link" ]]; then
        continue
    fi
    case "$python_link" in
        "$python_root"/lib/python*/config-*/libpython*.a|\
        "$python_root"/lib/python*/config-*/libpython*.dylib)
            rm -- "$python_link"
            echo "Omitted dangling optional Python development link: ${python_link#"$python_root"/}"
            ;;
        *)
            echo "Unexpected dangling Python runtime link: $python_link" >&2
            exit 1
            ;;
    esac
done < <(find "$python_root" -type l -print0)

dangling_link=$(find -L "$contents" -type l -print -quit)
if [[ -n "$dangling_link" ]]; then
    echo "Unexpected dangling symlink in app bundle: $dangling_link" >&2
    exit 1
fi

# The installed prefix's app RPATH points at its sibling lib directory.
# Add the bundle-relative Frameworks directory; the stager below then proves
# every non-system dependency resolves inside Contents.
"$install_name_tool_bin" -add_rpath "@executable_path/../Frameworks" "$macos/librecad"

# Qt's deployment utility copies Qt frameworks and the platform/image plugins.
# Third-party dependencies are already staged from the install prefix; rerun
# the closure walker after deployment to validate and re-sign the complete app.
"$macdeployqt_bin" "$bundle" -always-overwrite -no-strip -no-codesign -verbose=2

# The Cocoa plugin is unsuitable for headless CI smoke runs. Ship the
# offscreen plugin from the same Qt installation as macdeployqt so smoke tests
# exercise the bundle's Qt runtime rather than injecting external plugins.
qt_plugins=$($qtpaths_bin --query QT_INSTALL_PLUGINS)
offscreen_plugin="$qt_plugins/platforms/libqoffscreen.dylib"
if [[ ! -f "$offscreen_plugin" ]]; then
    echo "Required Qt offscreen platform plugin is unavailable: $offscreen_plugin" >&2
    exit 1
fi
mkdir -p "$contents/PlugIns/platforms"
ditto "$offscreen_plugin" "$contents/PlugIns/platforms/libqoffscreen.dylib"

# macdeployqt deploys optional Qt plugins from its installation even when the
# application does not use them. These plugins require Qt modules that are not
# part of LibreCAD's runtime (Qt PDF and Qt Virtual Keyboard); retaining them
# would leave unresolved loads in an otherwise closed bundle. Keep the Cocoa
# platform plugin and the image formats used by LibreCAD, including SVG.
for optional_plugin in \
    "imageformats/libqpdf.dylib" \
    "platforminputcontexts/libqtvirtualkeyboardplugin.dylib"; do
    plugin_path="$contents/PlugIns/$optional_plugin"
    if [[ -f "$plugin_path" ]]; then
        rm -- "$plugin_path"
        echo "Omitted unused optional Qt plugin: $optional_plugin"
    fi
done

"$python_bin" "$script_dir/stagePythonDependencies.py" \
    "$contents" \
    "$resources/librecad/python" \
    "$frameworks" \
    "$otool_bin" \
    "$install_name_tool_bin" \
    "$codesign_bin" \
    --application-binary "$macos/librecad" \
    --bundle-info-plist "$contents/Info.plist" \
    --scan-frameworks \
    --scan-directory "$contents/PlugIns"

# Seal the final resource tree only after plugin pruning, install-name changes,
# and dependency staging have completed.
"$codesign_bin" --force --deep --sign - "$bundle"
"$codesign_bin" --verify --deep --strict "$bundle"
echo "Created and verified macOS app bundle: $bundle"
