SplineOperations = {}
SplineOperations.__index = SplineOperations

setmetatable(SplineOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function SplineOperations:_init(id)
    Operations._init(self, id)

    self.points = {}

    self.spline_id = ID():id()
    self.spline = self:getSpline({Coord(0,0)})

    event.register('point', self)
    event.register('mouseMove', self)
    event.register('number', self)

    message("Add a new point")
end

function SplineOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point") then
        self:newPoint(...)
    elseif(eventName == "mouseMove") then
        self:createTempSpline(...)
    end
end

function SplineOperations:newPoint(point)
    table.insert(self.points, point)
end

function SplineOperations:getSpline(points)
    local d = active_widget():document()
    local layer = active_layer()
    local s = Spline(points, {}, {}, 1, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, layer, MetaInfo())
    s:setId(self.spline_id)

    return s
end

function SplineOperations:createTempSpline(point)
    active_widget():tempEntities():removeEntity(self.spline)

    local points = {}
    for k, v in pairs(self.points) do
        points[k] = v
    end
    table.insert(points, point)

    self.spline = self:getSpline(points)

    active_widget():tempEntities():addEntity(self.spline)
end

function SplineOperations:createSpline()
    local b = Builder(active_widget():document())
    local s = self:getSpline(self.points)
    b:append(s)
    b:execute()

    active_widget():tempEntities():removeEntity(self.spline)
    self.finished = true

    event.delete('mouseMove', self)
    event.delete('number', self)
    event.delete('point', self)
end

function SplineOperations:close()
    if(not self.finished) then
        if(#self.points > 1) then
            self:createSpline()
        else
            active_widget():tempEntities():removeEntity(self.spline)
            self.finished = true

            event.delete('mouseMove', self)
            event.delete('number', self)
            event.delete('point', self)
        end
    end
end