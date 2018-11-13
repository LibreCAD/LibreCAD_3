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
    self.builder = lc.builder.ArcBuilder()
    self.builder:setRadius(10)
    self.builder:setStartAngle(0)
    self.builder:setEndAngle(math.pi)
    self.step = 0

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
    if(self.step == 0) then
        self.builder:setCenter(point)

        message("Click on second point or enter the radius", self.target_widget)
        self.step = self.step + 1
    elseif(self.step == 1) then
        self.builder:setRadius(Operations:getDistance(self.builder:center(), point))

        message("Click on start point or enter the start angle", self.target_widget)
        self.step = self.step + 1
    elseif(self.step == 2) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), point))

        message("Click on end point or enter the end angle", self.target_widget)
        self.step = self.step + 1
    else
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), point))

        self:createArc()
    end
end

function ArcOperations:getArc()
    self.builder:setLayer(active_layer(self.target_widget))
    self.builder:setMetaInfo(active_metaInfo(self.target_widget))

    return self.builder:build()
end

function ArcOperations:createTempArc(point)
    if(self.step == 0) then
        self.builder:setCenter(point)
    elseif(self.step == 1) then
        self.builder:setRadius(Operations:getDistance(self.builder:center(), point))
    elseif(self.step == 2) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), point))
    else
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), point))
    end

    self.entity = self:getArc()
    self:refreshTempEntity()
end

function ArcOperations:createArc()
    local a = self:getArc()
    self:createEntity(a)

    CreateOperations.close(self)
end
