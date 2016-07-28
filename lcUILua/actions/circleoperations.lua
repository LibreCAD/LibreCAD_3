CircleOperations = {}
CircleOperations.__index = CircleOperations

setmetatable(CircleOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function CircleOperations:_init(id)
    Operations._init(self, id)

    self.center = nil
    self.circle = nil
    self.circle_id = ID():id()

    event.register('point', self)

    message("Click on center")
end

function CircleOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point") then
        self:newPoint(...)
    elseif(eventName == "mouseMove") then
        self:createTempCircle(...)
    elseif(eventName == "number") then
        self:createCircle(...)
    end
end

function CircleOperations:newPoint(point)
    if(self.center == nil) then
        self.center = point
        self.circle = self:getCircle(point, 0)

        active_widget():tempEntities():addEntity(self.circle)

        event.register('mouseMove', self)
        event.register('number', self)

        message("Click on second point or enter the radius")
    else
        self:createCircle(point)
    end
end

function CircleOperations:getCircle(center, radius)
    if(type(radius) == "userdata") then
        radius = center:distanceTo(radius)
    end

    local layer = active_layer()
    local metaInfo = active_metaInfo()
    local c = Circle(center, radius, layer, metaInfo)
    c:setId(self.circle_id)

    return c
end

function CircleOperations:createTempCircle(point)
    active_widget():tempEntities():removeEntity(self.circle)

    self.circle = self:getCircle(self.center, point)

    active_widget():tempEntities():addEntity(self.circle)
end

function CircleOperations:createCircle(point)
    self.finished = true
    active_widget():tempEntities():removeEntity(self.circle)

    local b = Builder(active_widget():document())
    local c = self:getCircle(self.center, point)
    b:append(c)
    b:execute()

    event.delete('mouseMove', self)
    event.delete('number', self)
    event.delete('point', self)
end

function CircleOperations:close()
    if(not self.finished) then
        active_widget():tempEntities():removeEntity(self.circle)
        self.finished = true

        event.delete('mouseMove', self)
        event.delete('number', self)
        event.delete('point', self)
    end
end