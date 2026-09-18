# DWG support in LibreCAD 3

**LibreCAD 3 reads DWG. It does not write DWG, and this is a decision rather
than a gap waiting to be filled.**

Reading is behind `WITH_DWG_IMPORT`, off by default, through libdxfrw's `dwgRW`.
Writing is not implemented, not exposed, and not planned until the conditions
below are met.

## Why reading is not symmetric with writing

DXF is a published interchange format. DWG is Autodesk's native format, and
every independent implementation of it is a reconstruction. Reading a
reconstruction wrong produces a drawing that looks wrong, which the user sees
and can reject. Writing one wrong produces a file other software accepts and
misinterprets — a drawing that is silently not what it claims to be, in a format
whose whole purpose is being handed to someone else.

That asymmetry is why the same library can be trusted for one direction and not
the other at the same moment in its life.

## The evidence, as of the pinned libdxfrw

The library tracks its own DWG format claims in
`third_party/libdxfrw/metadata/qualified-format-status-v1.json`. At the pinned
revision it records **15 claims: 11 `EXPERIMENTAL` and 4 `PENDING_NATIVE`. None
is `PROMOTED`.**

Every `EXPERIMENTAL` claim carries the same two blockers:

- `reviewed-target-debt`
- `unresolved-integrity-diagnostic`

The file's own transition rules allow `PENDING_NATIVE` → `PROMOTED`, but not
`EXPERIMENTAL` → `PROMOTED`: a claim must clear its blockers before it can even
be considered. The file's `freezeState` is `FROZEN`.

So the library does not claim its DWG writing is qualified, and LibreCAD should
not claim it either.

## What would reopen this

Three gates, all three, not any one:

1. **The library promotes its write claims.** `claimStatus` shows `PROMOTED`
   entries for the versions LibreCAD would offer, with native receipts
   attached — not merely `PENDING_NATIVE`, and not with blockers outstanding.

2. **Round-trip evidence against software LibreCAD does not control.** A file
   LibreCAD wrote, opened by at least one independent implementation, with the
   drawing intact. This is a local acceptance step: the tooling is proprietary
   and cannot run on CI runners, so it is a human gate and should stay one.

3. **A decision about what a wrong DWG costs.** Reading badly is visible.
   Writing badly is not. Whoever turns writing on is deciding that LibreCAD's
   DWG output can be handed to someone else's software, and that decision
   belongs to the project, not to whoever happens to land the code.

Until then, a drawing opened from a DWG is saved as DXF. `DocumentSource`
enforces that rather than documenting it: the DWG variant is marked
non-writable, so Save cannot write back over the file it came from and asks for
a new one.

## Keeping this honest

`scripts/ci/check-dwg-write-position.sh` asserts the position still holds and
reports the library's current claim statuses, so the day gate 1 moves is visible
rather than something someone notices years later.
