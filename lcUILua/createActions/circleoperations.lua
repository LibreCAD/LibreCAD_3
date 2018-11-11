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

    CreateOperations._init(self, id)
    message("Click on center", self.target_widget)
end

function CircleOperations:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(eventName == "point") then
        self:newPoint(data["position"])
    elseif(eventName == "mouseMove") then
        self:createTempCircle(data["position"])
    elseif(eventName == "number") then
        self:createCircle(data["number"])
    end
end

function CircleOperations:newPoint(point)
    if(self.center == nil) then
        self.center = point

        message("Click on second point or enter the radius", self.target_widget)
    else
        self:createCircle(point)
    end
end

function CircleOperations:getCircle(center, radius)
    if(type(radius) == "userdata") then
        radius = center:distanceTo(radius)
    end

    local layer = active_layer(self.target_widget)
    local metaInfo = active_metaInfo(self.target_widget)
    local c = lc.entity.Circle(center, radius, layer, metaInfo, nil)

    return c
end

function CircleOperations:createTempCircle(point)
    if(self.center ~= nil) then
        self.entity = self:getCircle(self.center, point)
        self:refreshTempEntity()
    end
end

function CircleOperations:createCircle(point)
    local c = self:getCircle(self.center, point)
    self:createEntity(c)

    CreateOperations.close(self)
end