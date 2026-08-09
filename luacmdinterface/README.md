LibreCAD 3 Command Line Interface
==========

This tool runs a scripting-language snippet against an in-memory
LibreCAD document and renders the result to a TGA image.  Two runtimes
are supported (phase 2 of the Python-scripting plan added .py — see
`/Users/dli/.claude/plans/librecad3-python-scripting-2026-08-09.md`).

Usage
-----

```
./luacmdinterface -i file:test.lua -o test.tga
./luacmdinterface -i file:test.py  -o test.tga     # requires WITH_PYTHONSCRIPT=ON
```

Note: `-o test.png` (or any non-TGA extension) is IGNORED — the writer
is always uncompressed 24-bit TGA regardless of the `-t` option value
(pre-existing bug, phase-2 sub-plan optional fix-up).  On Linux CI the
GL context requires `xvfb-run -a ./luacmdinterface ...`.

Runtime dispatch
----------------

The chooser (`dispatch.h`, added in phase 2 slice 2.1) strips URL
`?query` and `#fragment` before matching the suffix, so
`file:test.py?flavor=old` correctly picks Python.  Suffix match is
case-insensitive.

The Python arm creates a per-invocation `PyNamespace`, injects the
document under the name `document` (exact parity with Lua's
`setDocument(document)`), executes the script via `LCPython::runString`,
and returns exit code 2 with the traceback on stderr on error — same
contract as the Lua arm.

Parity check (slice 2.4)
------------------------

Both `test.lua` and `test.py` produce the SAME geometry: a spiral of
lines (spacing `d += 0.05` up to `d < 8*pi`), one circle at (200, 200)
r = 30, one half-circle arc at (-100, -100) r = 40.  This is by
design: any divergence between the two after a bindings change is
regression evidence.

**Primary parity check** (entity-count + coordinate diff, robust
across GL drivers): the `parity_test` gtest under
`unittest/python/parity_test.cpp` dumps `test.py`'s entity list in a
diffable text format and, if `LIBRECAD_PARITY_OUTFILE=<path>` is set,
writes it there for comparison.  The matching Lua-side dump is
produced by a small runner script (planned for CI as part of the
phase-2 slice-2.6 persistence bindings; runs `luacmdinterface -i
file:test.lua` under xvfb and dumps the resulting Document via
`entityContainer():asVector()`).  A CI job then `diff -u`s the two
files.

Format: one line per entity, `<class>\t<field>=<value>...`,
sorted lexicographically.  Coordinates are printed at 6 fixed
fractional digits (both languages use the same C++ formatter so no
locale drift).

**Secondary parity check** (same-machine, manual, fragile across GL
drivers): render both runtimes to TGA and byte-compare:

```bash
./luacmdinterface -i file:test.lua -o out.lua.tga
./luacmdinterface -i file:test.py  -o out.py.tga
cmp out.lua.tga out.py.tga
```

Fragility warning: pixel output depends on the OpenGL driver's
rasterization decisions — the `cmp` may fail even when geometry is
identical.  Use the primary entity-dump diff for regression gating;
keep the TGA compare as a manual eyeball check.

TODO
==========

- Fix the fType/PNG-output bug (write PNG via libpng which lcviewernoqt
  already links, or drop `-t` and document TGA — phase-2 sub-plan
  optional item).
- A `--script foo.py` argument on the `librecad` binary itself
  (greenfield; not planned in any phase).
