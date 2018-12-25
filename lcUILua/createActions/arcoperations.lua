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
    CreateOperations._init(self, widget, lc.builder.ArcBuilder, "enterCenter")
    self.builder:setRadius(10)
    self.builder:setStartAngle(0)
    self.builder:setEndAngle(math.pi)

    message("Click on center", widget)
end

function ArcOperations:enterCenter(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setCenter(data["position"])
    end

    if(eventName == "point") then
        message("Click on second point or enter the radius", self.target_widget)
        self.step = "setRadius"
    end
end

function ArcOperations:setRadius(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setRadius(Operations:getDistance(self.builder:center(), data["position"]))
    elseif(eventName == "number") then
        self.builder:setRadius(data["number"])
    end

    if(eventName == "point" or eventName == "number") then
        message("Click on start point or enter the start angle", self.target_widget)
        self.step = "setStartAngle"
    end
end

function ArcOperations:setStartAngle(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), data["position"]))
    elseif(eventName == "number") then
        self.builder:setStartAngle(data["number"])
    end

    if(eventName == "point" or eventName == "number") then
        message("Click on end point or enter the end angle", self.target_widget)
        self.step = "setEndAngle"
    end
end

function ArcOperations:setEndAngle(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))
    elseif(eventName == "number") then
        self.builder:setEndAngle(data["number"])
    end

    if(eventName == "point" or eventName == "number") then
        self:createEntity()
    end
end