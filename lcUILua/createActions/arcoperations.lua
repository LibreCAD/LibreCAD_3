ArcOperations = {}
ArcOperations.__index = ArcOperations

setmetatable(ArcOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function ArcOperations:_init(widget)
    self.center = nil
    self.radius = nil
    self.beginAngle = nil
    self.endAngle = nil
    self.entity_id = ID():id()

    CreateOperations._init(self, widget)

    message("Click on center", widget)
end

function ArcOperations:onEvent(eventName, event)
    if(Operations.forMe(self, event) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(event["position"])
    elseif(eventName == "mouseMove") then
        self:createTempArc(event["position"])
    end
end

function ArcOperations:newData(point)
    if(self.center == nil) then
        self.center = point

        message("Click on second point or enter the radius", self.target_widget)
    elseif(self.radius == nil) then
        self.radius = Operations:getDistance(self.center, point)
        message("Click on start point or enter the start angle", self.target_widget)
    elseif(self.beginAngle == nil) then
        self.beginAngle = Operations:getAngle(self.center, point)
        message("Click on end point or enter the end angle", self.target_widget)
    else
        self.endAngle = Operations:getAngle(self.center, point)
        self:createArc()
    end
end

function ArcOperations:getArc(center, radius, beginAngle, endAngle)
    local layer = active_layer(self.target_widget)
    local metaInfo = active_metaInfo(self.target_widget)
    local a = Arc(center, radius, beginAngle, endAngle, false, layer, metaInfo)
    a:setId(self.entity_id)

    return a
end

function ArcOperations:createTempArc(point)
    local center = self.center
    local radius = self.radius
    local beginAngle = self.beginAngle
    local endAngle = self.endAngle

    if(center == nil) then
        center = point
    elseif(radius == nil) then
        radius = Operations:getDistance(center, point)
    elseif(beginAngle == nil) then
        beginAngle = Operations:getAngle(center, point)
    elseif(endAngle == nil) then
        endAngle = Operations:getAngle(center, point)
    end

    radius = radius or 0
    beginAngle = beginAngle or 0
    endAngle = endAngle or 0.5 * math.pi

    self.entity = self:getArc(center, radius, beginAngle, endAngle)
    self:refreshTempEntity()
end

function ArcOperations:createArc()
    local a = self:getArc(self.center, self.radius, self.beginAngle, self.endAngle)
    self:createEntity(a)

    CreateOperations.close(self)
end
