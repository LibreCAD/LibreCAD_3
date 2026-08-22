# luacmdinterface/test.py — modern-API demo, phase 2 slice 2.3.
#
# Run headlessly with (xvfb-run required on Linux CI — GL context is real):
#   ./luacmdinterface -i file:test.py -o out.tga
#
# Draws a spiral of lines + one circle + one arc so the parity check (slice
# 2.4) has non-trivial geometry to compare against test.lua.  Uses only
# APIs exposed by the phase-1 kernel bindings — no gui.*, no operations
# module, no path.lua (those land in phases 3+, and this script must run in
# the headless CLI where no GUI exists).
#
# The `document` global is injected by luacmdinterface (phase 2 slice 2.1),
# in exact parity with the Lua side's setDocument.  There is no
# `mainWindow` here — the CLI never had one.

import math

# ---- Layer + line-pattern (any drawing needs a Layer) -----------------------
# Layer signature: (name, MetaLineWidthByValue, Color, DxfLinePatternByValue,
#                   isFrozen).  Same defaults kaguya uses on the Lua side.
layer = lc.meta.Layer(
    "0",
    lc.meta.MetaLineWidthByValue(1.0),
    lc.Color(255, 255, 255, 255),
    None,
    False,
)

# The Layer must exist as a *document* meta type before it can be referenced
# by an entity that gets inserted, so we run an AddLayer op first.  In the
# Lua side this is the ubiquitous `AddLayer(document, layer):execute()` idiom.
add_layer_op = lc.operation.AddLayer.new(document, layer)
add_layer_op.execute()

# ---- Draw a spiral of lines -------------------------------------------------
# Same geometry as test.lua so slice 2.4's parity check has 1:1 entity counts.
rx, ry, r = 100.0, 100.0, 10.0

# EntityBuilder is the batch-append operation — one commit for the whole
# spiral, so undo works as a single step.  Same as
# `Builder(document, "spiral")` on the Lua side (which is `lc.operation.Builder`).
eb = lc.operation.EntityBuilder.new(document)

d = 0.0
prev_x, prev_y = rx, ry
count = 0
while d < 8 * math.pi:
    x = rx + (math.sin(d) * d) * r
    y = ry + (math.sin(d + (math.pi / 2)) * (d + (math.pi / 2))) * r
    if d > 0:
        lb = lc.builder.LineBuilder()
        lb.setLayer(layer)
        lb.setStartPoint(lc.geo.Coordinate(prev_x, prev_y))
        lb.setEndPoint(lc.geo.Coordinate(x, y))
        eb.appendEntity(lb.build())
        count += 1
    prev_x, prev_y = x, y
    d += 0.05  # coarser step than test.lua's 0.005 so the run stays snappy;
               # test.lua uses the same step so parity holds.

eb.execute()

# ---- One circle + one arc for extra coverage --------------------------------
# CircleBuilder / ArcBuilder use the same builder shape (setCenter, setRadius,
# +setStart/setEnd for arcs).
cb = lc.builder.CircleBuilder()
cb.setLayer(layer)
cb.setCenter(lc.geo.Coordinate(200.0, 200.0))
cb.setRadius(30.0)

ab = lc.builder.ArcBuilder()
ab.setLayer(layer)
ab.setCenter(lc.geo.Coordinate(-100.0, -100.0))
ab.setRadius(40.0)
ab.setStartAngle(0.0)
ab.setEndAngle(math.pi)
ab.setIsCCW(True)

# Commit both in a single EntityBuilder pass (matches test.lua's style).
eb2 = lc.operation.EntityBuilder.new(document)
eb2.appendEntity(cb.build())
eb2.appendEntity(ab.build())
eb2.execute()

# ---- Report -----------------------------------------------------------------
print("test.py: added {} lines + 1 circle + 1 arc".format(count + 2))
