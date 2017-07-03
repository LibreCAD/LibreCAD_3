LineOperations = {}
LineOperations.__index = LineOperations

setmetatable(LineOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function LineOperations:_init(id)
    Operations._init(self, id)

    self.lastPoint = nil
    self.length = nil
    self.line_id = ID():id()
    self.lastLine = self:createLine(Coord(0, 0), Coord(0, 0))
    active_widget():tempEntities():addEntity(self.lastLine)

    event.register('point', self)

    message("Click on first point")
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
        active_widget():tempEntities():removeEntity(self.lastLine)

        local b = EntityBuilder(active_widget():document())
        local l = self:createLine(self.lastPoint, point)
        b:appendEntity(l)
        b:execute()

        event.delete('mouseMove', self)
        event.delete('number', self)
        event.delete('point', self)
    else
        self.lastPoint = point
        self.lastLine = self:createLine(point, point)

        event.register('mouseMove', self)
        event.register('number', self)
        message("Click on second point or enter line length")
    end
end

function LineOperations:createTempLine(point)
    active_widget():tempEntities():removeEntity(self.lastLine)

    self.lastLine = self:createLine(self.lastPoint, point)
    active_widget():tempEntities():addEntity(self.lastLine)
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
    l:setId(self.line_id)

    return l
end

function LineOperations:close()
    if(not self.finished) then
        active_widget():tempEntities():removeEntity(self.lastLine)
        self.finished = true

        event.delete('mouseMove', self)
        event.delete('number', self)
        event.delete('point', self)
    end
end