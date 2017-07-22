DimAngularOperations = {}
DimAngularOperations.__index = DimAngularOperations

setmetatable(DimAngularOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimAngularOperations:_init(id)
    Operations._init(self, id)

    self.centerPoint = nil
    self.firstPoint = nil
    self.secondPoint = nil
    self.text = nil

    self.dimAngular_id = ID():id()
    self.dimAngular = self:getDimAngular(Coord(0,0), Coord(1,0), Coord(1,1), "<>")
    self.dimLine = nil

    active_widget():tempEntities():addEntity(self.dimAngular)

    luaInterface:registerEvent('point', self)
    luaInterface:registerEvent('mouseMove', self)
    luaInterface:registerEvent('number', self)
    luaInterface:registerEvent('text', self)

    message("Click on center point")
end

function DimAngularOperations:getDimAngular(certerPoint, firstPoint, secondPoint, text)
    local layer = active_layer()
    local metaInfo = active_metaInfo()
    local dim = DimAngular.dimAuto(certerPoint, firstPoint, secondPoint, text, layer, metaInfo)
    dim:setId(self.dimAngular_id)

    return dim
end

function DimAngularOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:createTempDimAngular(...)
    elseif(eventName == "text") then
        self:setText(...)
    end
end

function DimAngularOperations:newData(data)
    if(self.centerPoint == nil) then
        self.centerPoint = Operations:getCoordinate(data)

        message("Click on first point")
    elseif(self.firstPoint == nil) then
        self.firstPoint = Operations:getCoordinate(data)

        message("Click on second point")
    elseif(self.secondPoint == nil) then
        self.secondPoint = Operations:getCoordinate(data)

        message("Enter dimension text or leave it empty (<> for value)")
        cli_get_text(true)
    end
end

function DimAngularOperations:setText(text)
    if(text == "") then
        self.text = "<>"
    else
        self.text = text
    end

    cli_get_text(false)
    self:createDimAngular()
end

function DimAngularOperations:createTempDimAngular(point)
    local centerPoint = self.centerPoint
    local firstPoint = self.firstPoint
    local secondPoint = self.secondPoint

    if(centerPoint == nil) then
        centerPoint = point
    elseif(firstPoint == nil) then
        firstPoint = point
    elseif(secondPoint == nil) then
        secondPoint = point
    end

    firstPoint = firstPoint or centerPoint:add(Coord(-1, 1))
    secondPoint = secondPoint or centerPoint:add(Coord(1, 1))

    active_widget():tempEntities():removeEntity(self.dimAngular)

    self.dimAngular = self:getDimAngular(centerPoint, firstPoint, secondPoint, "<>")

    active_widget():tempEntities():addEntity(self.dimAngular)
end

function DimAngularOperations:createDimAngular()
    self.finished = true
    active_widget():tempEntities():removeEntity(self.dimAngular)

    local b = EntityBuilder(active_widget():document())
    local c = self:getDimAngular(self.centerPoint, self.firstPoint, self.secondPoint, self.text)
    b:appendEntity(c)
    b:execute()

    luaInterface:deleteEvent('mouseMove', self)
    luaInterface:deleteEvent('number', self)
    luaInterface:deleteEvent('point', self)
    luaInterface:deleteEvent('text', self)
end

function DimAngularOperations:close()
    if(not self.finished) then
        if(self.centerPoint ~= nil and self.firstPoint ~= nil and self.secondPoint ~= nil) then
            self.text = "<>"
            self:createDimAngular()
        else
            active_widget():tempEntities():removeEntity(self.dimAngular)
            self.finished = true

            luaInterface:deleteEvent('mouseMove', self)
            luaInterface:deleteEvent('number', self)
            luaInterface:deleteEvent('point', self)
            luaInterface:deleteEvent('text', self)
        end
    end
end