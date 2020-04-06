LineOperations = {}
LineOperations.__index = LineOperations

setmetatable(LineOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function LineOperations:_init(id)
    CreateOperations._init(self, id, lc.builder.LineBuilder, "setFirstPoint")
    self.length = nil
end

function LineOperations:_init_default()
    message("<b>LINE</b>", self.target_widget)
    --message("Options: <b><u>C</u>ontinuous</b>, <u>S</u>egment", self.target_widget)  TODO: Multiple lines in single command.
    message("Click on first point or enter coordinates:", self.target_widget)
    self.step = "setFirstPoint"
end

function LineOperations:_init_2p()
    message("<b>LINE</b>", self.target_widget)
    --message("Options: <b><u>C</u>ontinuous</b>, <u>S</u>egment", self.target_widget)  TODO: Multiple lines in single command.
    message("Click on first point or enter coordinates:", self.target_widget)
    self.step = "setFirstPoint"
end

function LineOperations:_init_pal()
    message("<b>LINE</b>", self.target_widget)
    --message("Options: <b><u>C</u>ontinuous</b>, <u>S</u>egment", self.target_widget)  TODO: Multiple lines in single command.
    message("Click on first point or enter coordinates:", self.target_widget)
    self.step = "LineWithPointAngleLength"
end

function LineOperations:setFirstPoint(eventName, data)
    if(eventName == "point") then
        self.builder:setStartPoint(data["position"])
        message("Click on second point or enter coordinates or enter line length", self.target_widget)
        self.step = "setSecondPoint"
    end
end

function LineOperations:getSecondPoint(mousePos)
    if(self.length ~= nil) then
       local angle = self.builder:startPoint():angleTo(mousePos)
       local relativeCoordinate = lc.geo.Coordinate(angle):multiply(self.length)
       return self.builder:startPoint():add(relativeCoordinate)
    else
        return mousePos
    end
end

function LineOperations:setSecondPoint(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setEndPoint(self:getSecondPoint(data["position"]))
    end

    if(eventName == "point") then
        self:createEntity()

    elseif(eventName == "number") then
        if(self.length ~= nil) then
            local angle = math.rad(data["number"])
            local relativeCoordinate = lc.geo.Coordinate(angle):multiply(self.length)
            self.builder:setEndPoint(self.builder:startPoint():add(relativeCoordinate))
            self:createEntity()
            self:close()
        else
            self.length = data["number"]
            message("Click on second point or enter coordinates or enter line angle", self.target_widget)
        end
    end
end

function LineOperations:LineWithPointAngleLength(eventName, data)
    if(eventName == "point" and not self.startPoint) then
        self.builder:setStartPoint(data["position"])
        self.startPoint = data["position"]
        message("Click to select angle or enter angle (in degrees):", self.target_widget)
    elseif(eventName == "mouseMove" and self.startPoint and not self.angle) then
        local angle = self.builder:startPoint():angleTo(data["position"])
        local p2 = lc.geo.Coordinate(50,0)
        p2 = p2:rotate(angle)
        self.builder:setEndPoint(self.startPoint:add(p2))
    elseif(eventName == "point" and self.startPoint and not self.angle) then
        self.angle = self.builder:startPoint():angleTo(data["position"])
        local p2 = lc.geo.Coordinate(50,0)
        p2 = p2:rotate(self.angle)
        self.builder:setEndPoint(self.startPoint:add(p2))
        message("Enter length or click on point to at required length", self.target_widget)
    elseif(eventName == "number" and self.startPoint and not self.angle) then
        self.angle = data["number"] * (3.14159265) / 180
        local p2 = lc.geo.Coordinate(50,0)
        p2 = p2:rotate(self.angle)
        self.builder:setEndPoint(self.startPoint:add(p2))
        message("Enter length or click on point to at required length", self.target_widget)
    elseif(eventName == "mouseMove" and self.startPoint and self.angle) then
        local length = data["position"]:sub(self.startPoint):magnitude()
        local p2 = lc.geo.Coordinate(length,0)
        p2 = p2:rotate(self.angle)
        self.builder:setEndPoint(self.startPoint:add(p2))
    elseif(eventName == "point" and self.startPoint and self.angle) then
        local length = data["position"]:sub(self.startPoint):magnitude()
        local p2 = lc.geo.Coordinate(length,0)
        p2 = p2:rotate(self.angle)
        self.builder:setEndPoint(self.startPoint:add(p2))
        self:createEntity()
        self:close()
    elseif(eventName == "number" and self.startPoint and self.angle) then
        local length = data["number"]
        local p2 = lc.geo.Coordinate(length,0)
        p2 = p2:rotate(self.angle)
        self.builder:setEndPoint(self.startPoint:add(p2))
        self:createEntity()
        self:close()
    end
end

function LineOperations:manualClose() -- this function gets called after the first line is added to document
        local newBuilder = lc.builder.LineBuilder()
        newBuilder:setStartPoint(self.builder:endPoint())
        self.builder = newBuilder
        self.step = "setSecondPoint"    
end
