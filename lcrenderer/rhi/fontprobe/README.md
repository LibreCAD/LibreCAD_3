# R0 CPU font provider audit

This small native probe tests the existing FreeType candidate independently of
Qt, OpenGL, QRhi, scripting or a desktop session. It embeds the pinned ABeeZee font
and reads expected glyph IDs/design-unit metrics from the existing renderer
corpus at configure time. It does not add a second fixture registry.

```sh
cmake -S lcrenderer/rhi/fontprobe -B /absolute/new-font-build \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/absolute/freetype-prefix
cmake --build /absolute/new-font-build --parallel 3
ctest --test-dir /absolute/new-font-build --output-on-failure
/absolute/new-font-build/lcfontprobe > /absolute/font-probe.json
```

Use a matching prebuilt FreeType development/runtime provider, not its libtool
ABI/pkg-config number as an API version. The probe checks the actual compiled and
runtime API versions, uses target-local non-system imported includes to prevent
stale implicit headers winning, and records a source/font/notice/provider digest.
Keep the generated `source-inputs.sha256` and copied `ABeeZee-OFL.txt` with results.
`BUILD_TESTING=OFF` builds only the C++ audit without discovering a Python test host.

The native test covers H, é and the absent supplementary codepoint's glyph zero:
raw cmap/advance/bearing, unhinted 32/64-ppem advances, bounded gray bitmaps, corrupt
font rejection and owned CPU pixels after face/library release. The second CTest
checks a valid configuration and ten specific bad font/notice/oracle/path/input
configurations. Wrong failures cannot satisfy those negatives.

The JSON kind is `r0-font-probe`, explicitly not graphics or a production painter.
It does not prove UTF-8 decoding, shaping, font fallback selection, atlas budgets,
text placement, GPU glyph readback, platform acceptance or a complete F5 contract.
The corpus's future renderer cases and D3 approval remain mandatory.

FreeType's [glyph loading and units](https://freetype.org/freetype2/docs/reference/ft2-glyph_retrieval.html)
distinguish unscaled design units from 26.6 pixel advances; this audit keeps them
separate. Only the pinned scalable non-tricky font and gray positive-pitch raster
domain is audited here, not a general font decoder or production raster policy.
