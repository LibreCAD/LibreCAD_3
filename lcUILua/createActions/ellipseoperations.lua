EllipseOperations = {}
EllipseOperations.__index = EllipseOperations

setmetatable(EllipseOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function EllipseOperations:_init(id, isArc)
    self.isArc = isArc or false

    CreateOperations._init(self, id, lc.builder.EllipseBuilder, "setCenter")
    self.builder:setMajorPoint(lc.geo.Coordinate(10, 0, 0))
    self.builder:setMinorRadius(10)
    self.builder:setStartAngle(0)
    self.builder:setEndAngle(2*math.pi)
    self.builder:setIsReversed(false)

    message("Click on center", id)
end

function EllipseOperations:setCenter(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setCenter(data["position"])
    end

    if(eventName == "point") then
    self.step = "setMajorPoint"
        message("Click on major point", self.target_widget)
    elseif(eventName == "mouseMove") then
        self:refreshTempEntity()
    end
end

function EllipseOperations:setMajorPoint(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setMajorPoint(data["position"]:sub(self.builder:center()))
    end

    if(eventName == "point") then
        self.step = "setMinorRadius"
        message("Give minor radius", self.target_widget)
    elseif(eventName == "mouseMove") then
        self:refreshTempEntity()
    end
end

function EllipseOperations:setMinorRadius(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setMinorRadius(self.builder:center():distanceTo(data["position"]))
    elseif(eventName == "number") then
        self.builder:setMinorRadius(data)
    end

    if(eventName == "point" or eventName == "number") then
        if(not self.isArc) then
            self:createEntity()
        else
            message("Enter start angle", self.target_widget)
            self.step = "setStartAngle"
        end
    elseif(eventName == "mouseMove") then
        self:refreshTempEntity()
    end
end

function EllipseOperations:setStartAngle(eventName, data)
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

function EllipseOperations:setEndAngle(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))
    elseif(eventName == "number") then
        self.builder:setEndAngle(data["number"])
    end

    if(eventName == "point" or eventName == "number") then
        self:createEntity()
    end
end
