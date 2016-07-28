FractalTree = {}
FractalTree.__index = FractalTree

setmetatable(FractalTree, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function FractalTree:_init(id)
    Operations._init(self, id)

    self.angle = nil
    self.depth = nil

    event.register('number', self)
    event.register('point', self)

    message("Enter tree angle")
end

function FractalTree:onEvent(eventName, data)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "number") then
        if(self.angle == nil) then
            self.angle = data

            message("Enter tree depth")
        elseif(self.depth == nil) then
            self.depth = data

            message("Give origin point")
        end
    elseif(eventName == "point") then
        if(self.angle == nil or self.depth == nil) then
            return
        end

        local b = Builder(active_widget():document())
        FractalTree.drawTree(b, data:x(), data:y(), self.angle, self.depth)
        b:execute()

        self:close()
    end
end

function FractalTree.drawTree(b, x1, y1, angle, depth)
    if depth == 0 then  return end;

    local x2 = x1 +  (math.cos(math.rad(angle)) * depth * 10.0);
    local y2 = y1 + (math.sin(math.rad(angle)) * depth * 10.0);

    b:append(Line(Coord(x1, y1), Coord(x2, y2), active_layer()));
    FractalTree.drawTree(b, x2, y2, angle - 20, depth - 1);
    FractalTree.drawTree(b, x2, y2, angle + 20, depth - 1);
end

function FractalTree:close()
    if(not self.finished) then
        self.finished = true

        event.delete('number', self)
        event.delete('point', self)

        event.trigger("operationFinished")
    end
end

local tab = toolbar:tabByName("Quick Access")
local group = tab:addGroup("Fractal")

local fractalButton = create_button("Fractal tree")
tab:addButton(group, fractalButton, 0, 0, 1, 1)
luaInterface:luaConnect(fractalButton, "pressed()", function()
    new_operation()
    op[active_widget().id] = FractalTree(active_widget().id)
end)