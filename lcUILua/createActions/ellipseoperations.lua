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
    self.Axis_StartPoint = nil
    self.Axis_EndPoint = nil
    self.Axis_CenterPoint = nil
    self.rotation = false
    CreateOperations._init(self, id, lc.builder.EllipseBuilder, "EllipsewithAxisEnd")
    cli_get_text(self.target_widget, true) -- This command prevents the user from entering coordinates in the command line. But at the same time it is needed for receiving text options from user. Alternate method need to be worked out.

    message("<b>Ellipse</b>", self.target_widget)
    message("Options: <u>E</u>lliptical Arc, <u>C</u>enter", self.target_widget)
    message("Provide Axis start Point:", self.target_widget)
end

function EllipseOperations:EllipsewithAxisEnd(eventName, data)
    if(eventName == "text" and not self.Axis_StartPoint) then
        if (string.lower(data["text"]) == "c" or string.lower(data["text"]) == "center") then
            message("Provide Center Point:", self.target_widget)
            cli_get_text(self.target_widget, false)
            self.step = "EllipsewithCenter"
        else
            message("Invalid input:" .. data["text"] ,self.target_widget)
            message("Provide Axis start Point:", self.target_widget)
        end
    elseif (eventName == "point" and not self.Axis_StartPoint ) then
        self.Axis_StartPoint = data['position']
        message("Provide Axis end Point:",self.target_widget)
    elseif (eventName == "point" and self.Axis_StartPoint and not self.Axis_EndPoint) then
        self.Axis_EndPoint = data['position']
        self.builder:setCenter(self.Axis_StartPoint:mid(self.Axis_EndPoint))
        self.builder:setMajorPoint(self.Axis_EndPoint:sub(self.builder:center()))
        message("Options: <u>R</u>otation or",self.target_widget)
        message("Provide other axis end Point:",self.target_widget)
    elseif (eventName == "text" and self.Axis_StartPoint and self.Axis_EndPoint) then
        if (string.lower(data["text"]) == "r" or string.lower(data["text"]) == "rotation") then
            message("Specify rotation:", self.target_widget)
            self.rotation = true
            cli_get_text(self.target_widget, false)
        else
            message("Invalid input:" .. data["text"] ,self.target_widget)
            message("Provide other axis end Point:", self.target_widget)
        end
    elseif (eventName == "mouseMove" and self.Axis_StartPoint and self.Axis_EndPoint and self.rotation == false) then
        self.builder:setMinorRadius(self.builder:center():distanceTo(data["position"]))
    elseif (eventName == "point" and self.Axis_StartPoint and self.Axis_EndPoint and self.rotation == false) then
        self.builder:setMinorRadius(self.builder:center():distanceTo(data["position"]))
        self:createEntity()
    elseif (eventName == "mouseMove" and self.Axis_StartPoint and self.Axis_EndPoint and self.rotation == true) then
        self.builder:setMinorRadius ( math.cos(self.builder:center():angleTo(data["position"])) *  self.Axis_StartPoint:distanceTo(self.Axis_EndPoint) / 2)
    elseif (eventName == "point" and self.Axis_StartPoint and self.Axis_EndPoint and self.rotation == true) then
        self.builder:setMinorRadius ( math.cos(self.builder:center():angleTo(data["position"])) *  self.Axis_StartPoint:distanceTo(self.Axis_EndPoint) / 2)
        self:createEntity()
    elseif (eventName == "number" and self.Axis_StartPoint and self.Axis_EndPoint and self.rotation == true) then
        local angle = math.rad(data["number"])
        self.builder:setMinorRadius ( math.cos(angle) *  self.Axis_StartPoint:distanceTo(self.Axis_EndPoint) / 2)
        self:createEntity()
    end
end

function EllipseOperations:EllipsewithCenter(eventName, data)
    if (eventName == "point" and not self.Axis_CenterPoint ) then
        self.Axis_CenterPoint = data['position']
        self.builder:setCenter(self.Axis_CenterPoint)
        message("Provide Axis end Point:",self.target_widget)
    elseif (eventName == "point" and self.Axis_CenterPoint and not self.Axis_EndPoint) then
        self.Axis_EndPoint = data['position']
        self.builder:setMajorPoint(self.Axis_EndPoint:sub(self.builder:center()))
        message("Options: <u>R</u>otation or",self.target_widget)
        cli_get_text(self.target_widget, true)
        message("Provide other axis end Point:",self.target_widget)
    elseif (eventName == "text" and self.Axis_EndPoint) then
        if (string.lower(data["text"]) == "r" or string.lower(data["text"]) == "rotation") then
            message("Specify rotation:", self.target_widget)
            self.rotation = true
            cli_get_text(self.target_widget, false)
        else
            message("Invalid input:" .. data["text"] ,self.target_widget)
            message("Provide other axis end Point:", self.target_widget)
        end
    elseif (eventName == "mouseMove"  and self.Axis_EndPoint and self.rotation == false) then
        self.minRadius=self.builder:center():distanceTo(data["position"])
        self.builder:setMinorRadius(self.minRadius)
    elseif (eventName == "point" and self.Axis_EndPoint and self.rotation == false) then
        self.minRadius=self.builder:center():distanceTo(data["position"])
        self.builder:setMinorRadius(self.minRadius)
        self:createEntity()
    elseif (eventName == "mouseMove"  and self.Axis_EndPoint and self.rotation == true) then
        self.minRadius = math.cos(self.builder:center():angleTo(data["position"])) *  self.Axis_CenterPoint:distanceTo(self.Axis_EndPoint)
        self.builder:setMinorRadius (self.minRadius )
    elseif (eventName == "point" and self.Axis_EndPoint and self.rotation == true ) then
        self.minRadius = math.cos(self.builder:center():angleTo(data["position"])) *  self.Axis_CenterPoint:distanceTo(self.Axis_EndPoint)
        self.builder:setMinorRadius (self.minRadius )
        self:createEntity()
    elseif (eventName == "number" and self.Axis_EndPoint and self.rotation == true) then
        self.minRadius =  math.cos(math.rad(data["number"])) *  self.Axis_CenterPoint:distanceTo(self.Axis_EndPoint)
        self.builder:setMinorRadius(self.minRadius)
        self:createEntity()
    end
end


--[[
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
]]


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
