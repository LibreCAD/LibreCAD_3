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
    self.lastLine = nil
    self.line_id = ID():id()

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
    end
end

function LineOperations:newPoint(point)
    if(self.lastPoint ~= nil) then
        active_widget():tempEntities():removeEntity(self.lastLine)

        local b = Builder(active_widget():document())
        local l = self:createLine(self.lastPoint, point)
        b:append(l)
        b:execute()

        event.delete('mouseMove', self)
        event.delete('point', self)
    else
        self.lastPoint = point
        self.lastLine = self:createLine(point, point)

        active_widget():tempEntities():addEntity(self.lastLine)

        event.register('mouseMove', self)
        message("Click on second point")
    end
end

function LineOperations:createTempLine(point)
    active_widget():tempEntities():removeEntity(self.lastLine)

    self.lastLine = self:createLine(self.lastPoint, point)
    active_widget():tempEntities():addEntity(self.lastLine)
end

function LineOperations:createLine(p1, p2)
    local d = active_widget():document()
    local layer = d:layerByName("0")
    local l = Line(Coord(p1:x(), p1:y()), Coord(p2:x(), p2:y()), layer);
    l:setId(self.line_id)

    return l
end