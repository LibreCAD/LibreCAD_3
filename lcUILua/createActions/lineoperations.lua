LineOperations = {}
LineOperations.__index = LineOperations

setmetatable(LineOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function LineOperations:_init(id)
    self.lastPoint = nil
    self.length = nil
    self.entity_id = ID():id()
    self.entity = self:createLine(Coord(0, 0), Coord(0, 0))
    luaInterface:registerEvent('point', self)
    message("Click on first point")

    CreateOperations._init(self, id)
end

function LineOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point") then
        self:newPoint(...)
    elseif(eventName == "mouseMove") then
        self:createTempLine(...)
    elseif(eventName == "number") then
        self.length = ...
    end
end

function LineOperations:newPoint(point)
    if(self.lastPoint ~= nil) then
        self.finished = true
        self:removeTempEntity()

        local l = self:createLine(self.lastPoint, point)
        self:createEntity(l)
        self:unregisterEvents()

    else
        self.lastPoint = point
        self.entity = self:createLine(point, point)

        luaInterface:registerEvent('mouseMove', self)
        luaInterface:registerEvent('number', self)
        message("Click on second point or enter line length")
    end
end

function LineOperations:createTempLine(point)
    self.entity = self:createLine(self.lastPoint, point)
    self:refreshTempEntity()
end

function LineOperations:createLine(p1, p2)
    local layer = active_layer()
    local metaInfo = active_metaInfo()

    if(self.length ~= nil) then
        local angle = p1:angleTo(p2)
        local relativeCoordinate = Coordinate._fromAngle(angle):mulDouble(self.length)
        p2 = p1:add(relativeCoordinate)
    end

    local l = Line(p1, p2, layer, metaInfo)
    l:setId(self.entity_id)

    return l
end
