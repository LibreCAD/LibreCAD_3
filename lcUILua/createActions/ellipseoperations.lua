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

function EllipseOperations:init()
    mainWindow:connectMenuItem("actionEllipse_Axis", function() run_basic_operation(EllipseOperations) end)
    mainWindow:connectMenuItem("actionEllipse_Arc", function() run_basic_operation(EllipseOperations, "_init_arc") end)
    mainWindow:connectMenuItem("actionEllipse_FociPoints", function() run_basic_operation(EllipseOperations, "_init_foci") end)

    mainWindow:getToolbar():addButton("", ":/icons/ellipse.svg", "Creation", 1, 1, function() run_basic_operation(EllipseOperations) end, "Ellipse")

    mainWindow:getCliCommand():addCommand("ELLIPSE", function() run_basic_operation(EllipseOperations) end)
    mainWindow:getCliCommand():addCommand("ARCELLIPSE", function() run_basic_operation(EllipseOperations, "_init_arc") end)
end

function EllipseOperations:_init()
    self.isArc = isArc or false
    self.Axis_StartPoint = nil
    self.Axis_EndPoint = nil
    self.Axis_CenterPoint = nil
    self.rotation = false
    CreateOperations._init(self, lc.builder.EllipseBuilder, "EllipsewithAxisEnd")
    mainWindow:getCliCommand():returnText( true) -- This command prevents the user from entering coordinates in the command line. But at the same time it is needed for receiving text options from user. Alternate method need to be worked out.
end

function EllipseOperations:_init_default()
    message("<b>Ellipse</b>")
    message("Options: <u>E</u>lliptical Arc, <u>C</u>enter")
    message("Provide Axis start Point:")
    self.step = "EllipsewithAxisEnd"
end

function EllipseOperations:_init_arc()
    message("<b>Ellipse</b>")
    message("Options: <u>E</u>lliptical Arc, <u>C</u>enter")
    message("Provide Center Point:")
    self.step = "EllipsewithCenter"
end

function EllipseOperations:_init_foci()
    message("<b>Ellipse</b>")
    message("Provide first foci point:")
    self.step = "EllipseFociPoints"
end

function EllipseOperations:EllipsewithAxisEnd(eventName, data)
    if(eventName == "text" and not self.Axis_StartPoint) then
        if (string.lower(data["text"]) == "c" or string.lower(data["text"]) == "center") then
            message("Provide Center Point:")
            mainWindow:getCliCommand():returnText( false)
            self.step = "EllipsewithCenter"
        else
            message("Invalid input:" .. data["text"] )
            message("Provide Axis start Point:")
        end
    elseif (eventName == "point" and not self.Axis_StartPoint ) then
        self.Axis_StartPoint = data['position']
        message("Provide Axis end Point:")
    elseif (eventName == "point" and self.Axis_StartPoint and not self.Axis_EndPoint) then
        self.Axis_EndPoint = data['position']
        self.builder:setCenter(self.Axis_StartPoint:mid(self.Axis_EndPoint))
        self.builder:setMajorPoint(self.Axis_EndPoint:sub(self.builder:center()))
        message("Options: <u>R</u>otation or")
        message("Provide other axis end Point:")
    elseif (eventName == "text" and self.Axis_StartPoint and self.Axis_EndPoint) then
        if (string.lower(data["text"]) == "r" or string.lower(data["text"]) == "rotation") then
            message("Specify rotation:")
            self.rotation = true
            mainWindow:getCliCommand():returnText( false)
        else
            message("Invalid input:" .. data["text"] )
            message("Provide other axis end Point:")
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
        message("Provide Axis end Point:")
    elseif (eventName == "point" and self.Axis_CenterPoint and not self.Axis_EndPoint) then
        self.Axis_EndPoint = data['position']
        self.builder:setMajorPoint(self.Axis_EndPoint:sub(self.builder:center()))
        message("Options: <u>R</u>otation or")
        mainWindow:getCliCommand():returnText( true)
        message("Provide other axis end Point:")
    elseif (eventName == "text" and self.Axis_EndPoint) then
        if (string.lower(data["text"]) == "r" or string.lower(data["text"]) == "rotation") then
            message("Specify rotation:")
            self.rotation = true
            mainWindow:getCliCommand():returnText( false)
        else
            message("Invalid input:" .. data["text"] )
            message("Provide other axis end Point:")
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

function EllipseOperations:EllipseFociPoints(eventName, data)
    if (eventName == "point" and not self.Axis_FirstFoci ) then
        self.Axis_FirstFoci = data['position']
        message("Provide other foci point:")
    elseif (eventName == "point" and self.Axis_FirstFoci and not self.Axis_SecondFoci) then
        self.Axis_SecondFoci = data['position']
        self.builder:setCenter(self.Axis_FirstFoci:mid(self.Axis_SecondFoci))
        self.Axis_FociDistance = self.Axis_FirstFoci:distanceTo(self.builder:center())
        message("Provide major point:")
    elseif (eventName == "mouseMove" and self.Axis_FirstFoci and self.Axis_SecondFoci and not self.Axis_MajorPoint) then
        local majorPoint = data["position"]:sub(self.builder:center())
        if(majorPoint:magnitude() > self.Axis_FociDistance) then
            self.builder:setMajorPoint(majorPoint)
        else
            self.builder:setMajorPoint(self.Axis_FirstFoci:sub(self.builder:center()))
        end
        self.minRadius=self.builder:center():distanceTo(data["position"])
        self.builder:setMinorRadius(self.minRadius)
    elseif (eventName == "point" and self.Axis_FirstFoci and self.Axis_SecondFoci and not self.Axis_MajorPoint) then
        local majorPoint = data["position"]:sub(self.builder:center())
        if(majorPoint:magnitude() > self.Axis_FociDistance) then
            self.builder:setMajorPoint(majorPoint)
            self.Axis_MajorPoint = majorPoint
        else
            self.builder:setMajorPoint(self.Axis_FirstFoci:sub(self.builder:center()))
            self.Axis_MajorPoint = self.builder:majorPoint()
        end
        self.minRadius=self.builder:center():distanceTo(data["position"])
        self.builder:setMinorRadius(self.minRadius)
        message("Specify rotation:")
        mainWindow:getCliCommand():returnText( false)
    elseif (eventName == "mouseMove" and self.Axis_FirstFoci and self.Axis_SecondFoci and self.Axis_MajorPoint) then
        self.minRadius = self.builder:center():distanceTo(data["position"])
        self.builder:setMinorRadius(self.minRadius)
    elseif (eventName == "point" and self.Axis_FirstFoci and self.Axis_SecondFoci and self.Axis_MajorPoint) then
        self.minRadius = self.builder:center():distanceTo(data["position"])
        self.builder:setMinorRadius(self.minRadius)
        self:createEntity()
    elseif (eventName == "number" and self.Axis_FirstFoci and self.Axis_SecondFoci and self.Axis_MajorPoint) then
        self.minRadius =  math.cos(math.rad(data["number"])) *  self.builder:center():distanceTo(self.builder:center():add(self.builder:majorPoint()))
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
        message("Click on major point")
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
        message("Give minor radius")
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
            message("Enter start angle")
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
        message("Click on end point or enter the end angle")
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
