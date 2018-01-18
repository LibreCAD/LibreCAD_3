CircleOperations = {}
CircleOperations.__index = CircleOperations

setmetatable(CircleOperations, {
    __index = CreateOperations,
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

    luaInterface:registerEvent('point', self)

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

        luaInterface:registerEvent('mouseMove', self)
        luaInterface:registerEvent('number', self)

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

    local b = EntityBuilder(active_widget():document())
    local c = self:getCircle(self.center, point)
    b:appendEntity(c)
    b:execute()

    luaInterface:deleteEvent('mouseMove', self)
    luaInterface:deleteEvent('number', self)
    luaInterface:deleteEvent('point', self)
end

function CircleOperations:close()
    if(not self.finished) then
        active_widget():tempEntities():removeEntity(self.circle)
        self.finished = true

        luaInterface:deleteEvent('mouseMove', self)
        luaInterface:deleteEvent('number', self)
        luaInterface:deleteEvent('point', self)
    end
end