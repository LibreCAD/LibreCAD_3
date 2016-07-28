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
    self.degree = 3
    self.spline = self:getSpline({Coord(0,0)}, self.degree)

    event.register('point', self)
    event.register('mouseMove', self)
    event.register('number', self)

    message("Add a new points or enter degree")
end

function SplineOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point") then
        self:newPoint(...)
    elseif(eventName == "mouseMove") then
        self:createTempSpline(...)
    elseif(eventName == "number") then
        if(... >= 0 and ... <= 3) then
            self.degree = ...
        end
    end
end

function SplineOperations:newPoint(point)
    table.insert(self.points, point)
end

function SplineOperations:getSpline(points, degree)
    local layer = active_layer()
    local metaInfo = active_metaInfo()
    local s = Spline(points, {}, {}, degree, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, layer, metaInfo)

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

    self.spline = self:getSpline(points, self.degree)

    active_widget():tempEntities():addEntity(self.spline)
end

function SplineOperations:createSpline()
    local b = Builder(active_widget():document())
    local s = self:getSpline(self.points, self.degree)
    b:append(s)
    b:execute()
end

function SplineOperations:close()
    if(not self.finished) then
        if(#self.points > 1) then
            self:createSpline()
        end
        active_widget():tempEntities():removeEntity(self.spline)
        self.finished = true

        event.trigger('operationFinished')

        event.delete('mouseMove', self)
        event.delete('number', self)
        event.delete('point', self)
    end
end
