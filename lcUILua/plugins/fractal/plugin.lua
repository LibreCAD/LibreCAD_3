layer = Layer("Fractal", Color(255,255,255,255))
function drawTree( ce, x1,  y1,  angle,  depth)
    if depth == 0 then  return end;

    local x2 = x1 +  (math.cos(math.rad(angle)) * depth * 10.0);
    local y2 = y1 + (math.sin(math.rad(angle)) * depth * 10.0);


    ce:append(Line(Coord(x1, y1), Coord(x2, y2),layer));
    drawTree(ce, x2, y2, angle - 20, depth - 1);
    drawTree(ce, x2, y2, angle + 20, depth - 1);
end

local tab = toolbar:tabByName("Quick Access")
local group = tab:addGroup("Fractal")

local fractalButton = create_button("Fractal tree")
tab:addButton(group, fractalButton, 0, 0, 1, 1)
luaInterface:luaConnect(fractalButton, "pressed()", function()
    local b=Builder(active_widget():document())
    drawTree(b, 0, 0, 90, 14);
    b:execute()
end)