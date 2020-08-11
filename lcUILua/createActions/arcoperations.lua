ArcOperations = {
    name = "ArcOperations",
    command_line = "ARC",
    icon = "arc.svg",
    menu_actions = {
        p3 = "action3_Point_Arc",
        cse = "actionCenter_Start_End_2",
        csa = "actionCenter_Start_Angle_2",
        csl = "actionCenter_Start_Length_2",
        sce = "actionStart_Center_End_2",
        sca = "actionStart_Center_Angle_2",
        scl = "actionStart_Center_Length"
    },
    context_transitions = {
        ArcWithSCE = {"ArcWithSCA", "ArcWithSCL"},
        ArcWithSCA = {"ArcWithSCE", "ArcWithSCL"},
        ArcWithSCL = {"ArcWithSCE", "ArcWithSCA"},
        ArcWithCSE = {"ArcWithCSA", "ArcWithCSL"},
        ArcWithCSA = {"ArcWithCSE", "ArcWithCSL"},
        ArcWithCSL = {"ArcWithCSE", "ArcWithCSA"}
    }
}
ArcOperations.__index = ArcOperations

setmetatable(ArcOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function ArcOperations:_init()
    CreateOperations._init(self, lc.builder.ArcBuilder, "ArcWith3Points")
    self.builder:setRadius(10)
    self.Arc_FirstPoint = nil
    self.Arc_SecondPoint = nil
    self.Arc_ThirdPoint = nil
    self.Arc_Center = nil
end

function ArcOperations:_init_default()
    message("<b>Arc</b>")
    message("Options: <u>C</u>enter, or")
    message("Provide Start Point:")
	self.step = "ArcWith3Points"
end

function ArcOperations:_init_p3()
    message("<b>Arc - 3 point</b>")
    message("Provide Start Point:")
	self.step = "ArcWith3Points"
end

function ArcOperations:_init_cse()
    message("<b>Arc - Center Start End</b>")
    message("Provide Center Point:")
	self.step = "ArcWithCSE"
end

function ArcOperations:_init_csa()
    message("<b>Arc - Center Start Angle</b>")
    message("Provide Center Point:")
	self.step = "ArcWithCSA"
end

function ArcOperations:_init_csl()
    message("<b>Arc - Center Start Length</b>")
    message("Provide Center Point:")
	self.step = "ArcWithCSL"
end

function ArcOperations:_init_sce()
    message("<b>Arc - Start Center End</b>")
    message("Provide Start Point:")
	self.step = "ArcWithSCE"
end

function ArcOperations:_init_sca()
    message("<b>Arc - Start Center Angle</b>")
    message("Provide Start Point:")
	self.step = "ArcWithSCA"
end

function ArcOperations:_init_scl()
    message("<b>Arc - Start Center Angle</b>")
    message("Provide Start Point:")
	self.step = "ArcWithSCL"
end

function ArcOperations:ArcWith3Points(eventName, data)
    if(eventName == "text") then
        if (string.lower(data["text"]) == "c" or string.lower(data["text"]) == "center") then
            message("Provide Center Point:")
            self.step = "ArcWithCSE"
        else
            message("Invalid input:" .. data["text"] )
            message("Provide Radius:")
        end
    elseif (eventName == "point" and not self.Arc_FirstPoint) then
        self.Arc_FirstPoint = data['position']
        message("Provide Through Point:")
    elseif(eventName == "point" and self.Arc_FirstPoint and not self.Arc_SecondPoint) then
        message("Provide End Point:")
        self.Arc_SecondPoint = data['position']
    elseif(eventName == "mouseMove" and self.Arc_FirstPoint and self.Arc_SecondPoint and not self.Arc_ThirdPoint) then
        self.builder:setIsCCW(self:CheckCCW(self.Arc_FirstPoint, self.Arc_SecondPoint, data["position"]))
        self.builder:setCenter(self:Circumcenter(self.Arc_FirstPoint, self.Arc_SecondPoint, data['position']))
        self.builder:setRadius(self.builder:center():distanceTo(data['position']))
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))

    elseif(eventName == "point" and self.Arc_FirstPoint and self.Arc_SecondPoint and not self.Arc_ThirdPoint) then
        self.Arc_ThirdPoint = data['position']
        self.builder:setIsCCW(self:CheckCCW(self.Arc_FirstPoint, self.Arc_SecondPoint, self.Arc_ThirdPoint))
        self.builder:setCenter(self:Circumcenter(self.Arc_FirstPoint, self.Arc_SecondPoint, self.Arc_ThirdPoint))
        self.builder:setRadius(self.builder:center():distanceTo(self.Arc_ThirdPoint))
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))
        self:createEntity()
    end
end



function ArcOperations:ArcWithCSE(eventName, data) -- Create Arc with Center Start and End Points.
    if(eventName == "point" and not self.Arc_Center) then
        self.Arc_Center = data["position"]
        self.builder:setCenter(data["position"])
        message("Provide Start Point:")
    elseif(eventName == "point" and self.Arc_Center and not self.Arc_FirstPoint) then
        self.Arc_FirstPoint = data["position"]
        self.builder:setRadius(Operations:getDistance(self.builder:center(), data["position"]))
        message("Provide End Point:")
    elseif(eventName == "mouseMove" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_ThirdPoint) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))
    elseif(eventName == "point" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_ThirdPoint) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))
        self:createEntity()
    end
end

function ArcOperations:ArcWithSCE(eventName, data) -- Create Arc with Start, Center and End Points.
    if(eventName == "point" and not self.Arc_FirstPoint) then
        self.Arc_FirstPoint = data["position"]
        message("Provide Center Point:")
    elseif(eventName == "point" and self.Arc_FirstPoint and not self.Arc_Center) then
        self.Arc_Center = data["position"]
        self.builder:setCenter(data["position"])
        self.builder:setRadius(Operations:getDistance(data["position"], self.Arc_FirstPoint))
        message("Provide End Point:")
    elseif(eventName == "mouseMove" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_ThirdPoint) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))
    elseif(eventName == "point" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_ThirdPoint) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))
        self:createEntity()
    end
end

function ArcOperations:ArcWithSCA(eventName, data) -- Create Arc with Start, Center and Angle
    if(eventName == "point" and not self.Arc_FirstPoint) then
        self.Arc_FirstPoint = data["position"]
        message("Provide Center Point:")
    elseif(eventName == "point" and self.Arc_FirstPoint and not self.Arc_Center) then
        self.Arc_Center = data["position"]
        self.builder:setCenter(data["position"])
        self.builder:setRadius(Operations:getDistance(data["position"], self.Arc_FirstPoint))
        message("Enter angle:")
    elseif(eventName == "number" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_ThirdPoint) then
        local startAngle = Operations:getAngle(self.builder:center(), self.Arc_FirstPoint)
        local angleCalc = data["number"] * 3.1416/180
        self.builder:setStartAngle(startAngle)
        self.builder:setEndAngle((2 * 3.1415) + (startAngle + angleCalc))
        self:createEntity()
    elseif(eventName == "mouseMove" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_ThirdPoint) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        local angleCalc = self.Arc_Center:distanceTo(data["position"]) * 0.01
        self.builder:setEndAngle((2 * 3.1415) + Operations:getAngle(self.builder:center(), self.Arc_FirstPoint) + angleCalc)
    elseif(eventName == "point" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_ThirdPoint) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        local angleCalc = self.Arc_Center:distanceTo(data["position"]) * 0.01
        self.builder:setEndAngle((2 * 3.1415) + Operations:getAngle(self.builder:center(), self.Arc_FirstPoint) + angleCalc)
        self:createEntity()
    end
end

function ArcOperations:ArcWithSCL(eventName, data) -- Create Arc with Start, Center and Length
    if(eventName == "point" and not self.Arc_FirstPoint) then
        self.Arc_FirstPoint = data["position"]
        message("Provide Center Point:")
    elseif(eventName == "point" and self.Arc_FirstPoint and not self.Arc_Center) then
        self.Arc_Center = data["position"]
        self.builder:setCenter(data["position"])
        self.builder:setRadius(Operations:getDistance(data["position"], self.Arc_FirstPoint))
        message("Enter length:")
        mainWindow:cliCommand():returnText(false)
    elseif(eventName == "number" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_StartAngle) then
        local startAngle = Operations:getAngle(self.builder:center(), self.Arc_FirstPoint)
        local angleCalc = data["number"] / self.builder:radius()
        self.builder:setStartAngle(startAngle)
        self.builder:setEndAngle((2 * 3.1415) + startAngle + angleCalc)
        self:createEntity()
    elseif(eventName == "mouseMove" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_StartAngle) then
        local angleCalc = self.Arc_FirstPoint:distanceTo(data["position"]) / self.builder:radius()
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        self.builder:setEndAngle((2 * 3.1415) + Operations:getAngle(self.builder:center(), self.Arc_FirstPoint) + angleCalc)
    elseif(eventName == "point" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_StartAngle) then
        local angleCalc = self.Arc_FirstPoint:distanceTo(data["position"]) / self.builder:radius()
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        self.builder:setEndAngle((2 * 3.1415) + Operations:getAngle(self.builder:center(), self.Arc_FirstPoint) + angleCalc)
        self:createEntity()
    end
end

function ArcOperations:ArcWithCSA(eventName, data) -- Create Arc with Center Start and Angle.
    if(eventName == "point" and not self.Arc_Center) then
        self.Arc_Center = data["position"]
        self.builder:setCenter(data["position"])
        message("Provide Start Point:")
    elseif(eventName == "point" and self.Arc_Center and not self.Arc_FirstPoint) then
        self.Arc_FirstPoint = data["position"]
        self.builder:setRadius(Operations:getDistance(self.builder:center(), data["position"]))
        message("Enter Angle:")
    elseif(eventName == "number" and self.Arc_Center and self.Arc_FirstPoint) then
        local startAngle = Operations:getAngle(self.builder:center(), self.Arc_FirstPoint)
        local angleCalc = data["number"] * 3.1416/180
        self.builder:setStartAngle(startAngle)
        self.builder:setEndAngle((2 * 3.1415) + (startAngle + angleCalc))
        self:createEntity()
    elseif(eventName == "mouseMove" and self.Arc_Center and self.Arc_FirstPoint) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        local angleCalc = self.Arc_FirstPoint:distanceTo(data["position"]) * 0.01
        self.builder:setEndAngle((2 * 3.1415) + Operations:getAngle(self.builder:center(), self.Arc_FirstPoint) + angleCalc)
    elseif(eventName == "point" and self.Arc_Center and self.Arc_FirstPoint) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        local angleCalc = self.Arc_FirstPoint:distanceTo(data["position"]) * 0.01
        self.builder:setEndAngle((2 * 3.1415) + Operations:getAngle(self.builder:center(), self.Arc_FirstPoint) + angleCalc)
        self:createEntity()
    end
end

function ArcOperations:ArcWithCSL(eventName, data) -- Create Arc with Center, Start and Length
    if(eventName == "point" and not self.Arc_Center) then
        self.Arc_Center = data["position"]
        self.builder:setCenter(data["position"])
        message("Provide Start Point:")
    elseif(eventName == "point" and self.Arc_Center and not self.Arc_FirstPoint) then
        self.Arc_FirstPoint = data["position"]
        self.builder:setRadius(Operations:getDistance(self.builder:center(), data["position"]))
        message("Enter length:")
        mainWindow:cliCommand():returnText(false)
    elseif(eventName == "number" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_StartAngle) then
        local startAngle = Operations:getAngle(self.builder:center(), self.Arc_FirstPoint)
        local angleCalc = data["number"] / self.builder:radius()
        self.builder:setStartAngle(startAngle)
        self.builder:setEndAngle((2 * 3.1415) + startAngle + angleCalc)
        self:createEntity()
    elseif(eventName == "mouseMove" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_StartAngle) then
        local angleCalc = self.Arc_FirstPoint:distanceTo(data["position"]) / self.builder:radius()
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        self.builder:setEndAngle((2 * 3.1415) + Operations:getAngle(self.builder:center(), self.Arc_FirstPoint) + angleCalc)
    elseif(eventName == "point" and self.Arc_Center and self.Arc_FirstPoint and not self.Arc_StartAngle) then
        local angleCalc = self.Arc_FirstPoint:distanceTo(data["position"]) / self.builder:radius()
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), self.Arc_FirstPoint))
        self.builder:setEndAngle((2 * 3.1415) + Operations:getAngle(self.builder:center(), self.Arc_FirstPoint) + angleCalc)
        self:createEntity()
    end
end

function ArcOperations:Circumcenter(Point1,Point2,Point3)
    local Angle1=Point1:angleBetween(Point2,Point3)
    local Angle2=Point2:angleBetween(Point3,Point1)
    local Angle3=Point3:angleBetween(Point1,Point2)
    local X = (Point1:x() * math.sin(2 * Angle1) + Point2:x() * math.sin(2 * Angle2) + Point3:x() * math.sin(2 * Angle3) ) / ( math.sin(2 * Angle1) + math.sin(2 * Angle2) + math.sin(2 * Angle3))
    local Y = (Point1:y() * math.sin(2 * Angle1) + Point2:y() * math.sin(2 * Angle2) + Point3:y() * math.sin(2 * Angle3) ) / ( math.sin(2 * Angle1) + math.sin(2 * Angle2) + math.sin(2 * Angle3))
    local Output=lc.geo.Coordinate(X,Y)
    return Output
end

function ArcOperations:CheckCCW(P1,P2,P3)
    local K = ((P2:y() - P1:y()) * ( P3:x() - P2:x() ) ) - ( (P2:x() - P1:x() ) * ( P3:y() - P2:y() ) )
    if (K > 0) then return false else return true end
end
