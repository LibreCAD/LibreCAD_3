-- luacmdinterface/test.lua — modern-API demo, phase 2 slice 2.3.
-- Rewritten from the pre-slice-2.3 version, which used a long-removed flat
-- API (`Layer(...)`, `Coord(...)`, `Builder(...)`, `AddLayer(...)`).  The
-- old file didn't actually run against any release of the kernel bindings
-- — this rewrite makes it work AND makes it byte-comparable geometry to
-- test.py for the slice-2.4 parity check.
--
-- Run headlessly with (xvfb-run required on Linux CI — GL context is real):
--   ./luacmdinterface -i file:test.lua -o out.tga

-- ---- Layer + line-pattern (any drawing needs a Layer) ---------------------
local layer = lc.meta.Layer(
    "0",
    lc.meta.MetaLineWidthByValue(1.0),
    lc.Color(255, 255, 255, 255),
    nil,
    false
)

-- Register the layer as a document meta type via the AddLayer operation.
-- Same idiom kaguya-side scripts use throughout createActions/*.lua.
local addLayerOp = lc.operation.AddLayer.new(document, layer)
addLayerOp:execute()

-- ---- Draw a spiral of lines -----------------------------------------------
-- SAME geometry as test.py for the slice-2.4 parity check.
local rx, ry, r = 100.0, 100.0, 10.0
local eb = lc.operation.EntityBuilder.new(document)

local d = 0.0
local prev_x, prev_y = rx, ry
local count = 0
while d < 8 * math.pi do
    local x = rx + (math.sin(d) * d) * r
    local y = ry + (math.sin(d + (math.pi / 2)) * (d + (math.pi / 2))) * r
    if d > 0 then
        local lb = lc.builder.LineBuilder()
        lb:setLayer(layer)
        lb:setStartPoint(lc.geo.Coordinate(prev_x, prev_y))
        lb:setEndPoint(lc.geo.Coordinate(x, y))
        eb:appendEntity(lb:build())
        count = count + 1
    end
    prev_x, prev_y = x, y
    d = d + 0.05  -- same step as test.py so parity holds.
end

eb:execute()

-- ---- One circle + one arc for extra coverage ------------------------------
local cb = lc.builder.CircleBuilder()
cb:setLayer(layer)
cb:setCenter(lc.geo.Coordinate(200.0, 200.0))
cb:setRadius(30.0)

local ab = lc.builder.ArcBuilder()
ab:setLayer(layer)
ab:setCenter(lc.geo.Coordinate(-100.0, -100.0))
ab:setRadius(40.0)
ab:setStartAngle(0.0)
ab:setEndAngle(math.pi)
ab:setIsCCW(true)

local eb2 = lc.operation.EntityBuilder.new(document)
eb2:appendEntity(cb:build())
eb2:appendEntity(ab:build())
eb2:execute()

-- ---- Report ---------------------------------------------------------------
print("test.lua: added " .. tostring(count + 2) .. " entities (lines + 1 circle + 1 arc)")
