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

    luaInterface:registerEvent("point", self)
    luaInterface:registerEvent("number", self)

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

        local b = Builder(active_widget():document(), "Fractal tree")
        local block = Block("FractalTree_" .. math.random(9999999999), data) --TODO: get proper ID

        b:append(AddBlock(active_widget():document(), block))

        local eb = EntityBuilder(active_widget():document())
        b:append(eb)

        createFractalTree(eb, data:x(), data:y(), self.angle, self.depth, active_layer(), active_metaInfo(), block)

        local insertBuilder = InsertBuilder()
        insertBuilder:setLayer(active_layer())
        insertBuilder:setDisplayBlock(block)
        insertBuilder:setCoordinate(data)
        insertBuilder:setDocument(active_widget():document())
        eb:appendEntity(insertBuilder:build())

        b:execute()

        self:close()
    end
end

function createFractalTree(b, x1, y1, angle, depth, layer, metaInfo, block)
    if depth == 0 then  return end;

    local x2 = x1 +  (math.cos(math.rad(angle)) * depth * 10.0);
    local y2 = y1 + (math.sin(math.rad(angle)) * depth * 10.0);

    b:appendEntity(Line(Coord(x1, y1), Coord(x2, y2), layer, metaInfo, block));
    createFractalTree(b, x2, y2, angle - 20, depth - 1, layer, metaInfo, block);
    createFractalTree(b, x2, y2, angle + 20, depth - 1, layer, metaInfo, block);
end

function FractalTree:close()
    if(not self.finished) then
        self.finished = true

        luaInterface:deleteEvent('number', self)
        luaInterface:deleteEvent('point', self)

        luaInterface:triggerEvent("operationFinished", nil)
    end
end

if(LC_interface == "gui") then
    local tab = toolbar:tabByName("Quick Access")
    local group = tab:addGroup("Fractal")

    local fractalButton = create_button("Fractal tree")
    tab:addWidget(group, fractalButton, 0, 0, 1, 1)
    luaInterface:luaConnect(fractalButton, "pressed()", function()
        new_operation()
        luaInterface:setOperation(active_widget().id, FractalTree(active_widget().id))
    end)
end