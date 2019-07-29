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


function ArcOperations:_init(id)
    CreateOperations._init(self, id, lc.builder.ArcBuilder, "ArcWith3Points")
    cli_get_text(self.target_widget, true)
    self.builder:setRadius(10)
    self.Arc_FirstPoint = nil
    self.Arc_SecondPoint = nil
    self.Arc_ThirdPoint = nil
    self.Arc_Center = nil
    message("<b>Arc</b>", self.target_widget)
    message("Options: <u>C</u>enter, or", self.target_widget)
    message("Provide Start Point:", self.target_widget)
end

function ArcOperations:ArcWith3Points(eventName, data)
    if(eventName == "text") then
        if (string.lower(data["text"]) == "c" or string.lower(data["text"]) == "center") then
            message("Provide Center Point:", self.target_widget)
            self.step = "ArcWithCSE"
        else
            message("Invalid input:" .. data["text"] ,self.target_widget)
            message("Provide Radius:", self.target_widget)
        end
    elseif (eventName == "point" and not Arc_FirstPoint) then
        Arc_FirstPoint = data['position']
        message("Provide Through Point:",self.target_widget)
    elseif(eventName == "point" and Arc_FirstPoint and not Arc_SecondPoint) then
        message("Provide End Point:",self.target_widget)
        Arc_SecondPoint = data['position']
    elseif(eventName == "mouseMove" and Arc_FirstPoint and Arc_SecondPoint and  not Arc_ThirdPoint) then
        self.builder:setIsCCW(self:CheckCCW(Arc_FirstPoint,Arc_SecondPoint,data["position"]))
        self.builder:setCenter(self:Circumcenter(Arc_FirstPoint,Arc_SecondPoint,data['position']))
        self.builder:setRadius(self.builder:center():distanceTo(data['position']))
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), Arc_FirstPoint))
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))

    elseif(eventName == "point" and Arc_FirstPoint and Arc_SecondPoint and  not Arc_ThirdPoint) then
        Arc_ThirdPoint = data['position']
        self.builder:setIsCCW(self:CheckCCW(Arc_FirstPoint,Arc_SecondPoint,Arc_ThirdPoint))
        self.builder:setCenter(self:Circumcenter(Arc_FirstPoint,Arc_SecondPoint,Arc_ThirdPoint))
        self.builder:setRadius(self.builder:center():distanceTo(Arc_ThirdPoint))
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), Arc_FirstPoint))
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))
        self:createEntity()
    end
end



function ArcOperations:ArcWithCSE(eventName, data) -- Create Arc with Center Start and End Points.
    if(eventName == "point" and not Arc_Center ) then
        Arc_Center = data["position"]
        self.builder:setCenter(data["position"])
        message("Provide Start Point:", self.target_widget)
    elseif(eventName == "point" and  Arc_Center and not Arc_FirstPoint ) then
        Arc_FirstPoint = data["position"]
        self.builder:setRadius(Operations:getDistance(self.builder:center(), data["position"]))
        message("Provide End Point:", self.target_widget)
    elseif(eventName == "mouseMove" and  Arc_Center and Arc_FirstPoint and not Arc_ThirdPoint) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), Arc_FirstPoint))
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))
    elseif(eventName == "point" and  Arc_Center and Arc_FirstPoint and not Arc_ThirdPoint) then
        self.builder:setStartAngle(Operations:getAngle(self.builder:center(), Arc_FirstPoint))
        self.builder:setEndAngle(Operations:getAngle(self.builder:center(), data["position"]))
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
