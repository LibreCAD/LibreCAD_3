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
    self.center = nil
    self.entity = nil
    self.entity_id = ID():id()

    luaInterface:registerEvent('point', self)

    CreateOperations._init(self, id)
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
        self.entity = self:getCircle(point, 0)

        self:refreshTempEntity()

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
    c:setId(self.entity_id)

    return c
end

function CircleOperations:createTempCircle(point)
    self.entity = self:getCircle(self.center, point)
    self:refreshTempEntity()
end

function CircleOperations:createCircle(point)
    self.finished = true
    self:removeTempEntity()

    local c = self:getCircle(self.center, point)
    self:createEntity(c)

    self:unregisterEvents()
end