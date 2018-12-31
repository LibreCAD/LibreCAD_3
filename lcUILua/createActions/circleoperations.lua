CircleOperations = {}
CircleOperations.__index = CircleOperations
local Circle_FirstPoint
local Circle_SecondPoint
local Circle_ThirdPoint

setmetatable(CircleOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function CircleOperations:_init(id)
    CreateOperations._init(self, id, lc.builder.CircleBuilder, "enterCenter")
    cli_get_text(self.target_widget, true)
    self.builder:setRadius(10)
    Circle_FirstPoint = nil
    Circle_SecondPoint = nil
    Circle_ThirdPoint = nil
    message("<b>CIRCLE1</b>", self.target_widget)
    message("Options: <u>2</u>P, <u>3</u>P, <u>T</u>TT, TT<u>R</u>", self.target_widget)
    message("Provide Center Point:", self.target_widget)
end

function CircleOperations:enterCenter(eventName, data)
    if(eventName == "point") then
        self.builder:setCenter(data["position"])
        message("Options: <u>R</u>adius , <u>D</u>iameter", self.target_widget)
        message("Provide Radius:", self.target_widget)
        self.step = "enterRadius"
    elseif(eventName == "text") then
        if ((data["text"]) == "2" or string.lower(data["text"]) == "2p") then
            message("Provide Diameter Start Point:",self.target_widget)
            self.step = "CircleWith2Points"
        elseif ((data["text"]) == "3" or string.lower(data["text"]) == "3p") then
            message("Provide Frist Point:",self.target_widget)
            self.step = "CircleWith3Points"
        elseif (string.lower(data["text"]) == "t" or string.lower(data["text"]) == "ttt") then
            message("Provide Frist Tangent:",self.target_widget)
            self.step = "CircleWith3Tans"
        elseif (string.lower(data["text"]) == "r" or string.lower(data["text"]) == "ttr") then
            message("Provide Frist Tangent:",self.target_widget)
            self.step = "CircleWith2Tans"
        end
    end
end

function CircleOperations:enterRadius(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setRadius(self.builder:center():distanceTo(data["position"]))
    elseif(eventName == "text") then
        if (tonumber(data["text"]) == nil) then
            if (string.lower(data["text"]) == "d" or string.lower(data["text"]) == "diameter") then
                self.builder:setRadius(self.builder:radius() / 2)
                message("Provide Diameter:", self.target_widget)
                self.step = "enterDiameter"
            else
                message("Invalid input:" .. data["text"] ,self.target_widget)
                message("Provide Radius:", self.target_widget)
            end
        else
            message(data["text"],self.target_widget)
            self.builder:setRadius(tonumber(data["text"]))
            self:createEntity()
        end
    end
    if(eventName == "point") then
        self:createEntity()
    end
end

function CircleOperations:enterDiameter(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setRadius(self.builder:center():distanceTo(data["position"]) / 2)
    elseif(eventName == "text") then
        if (tonumber(data["text"]) == nil) then
            message("Invalid input:" .. data["text"] ,self.target_widget)
            message("Provide Diameter:", self.target_widget)
        else
            message(data["text"],self.target_widget)
            self.builder:setRadius(tonumber(data["text"])/2)
            self:createEntity()
        end
    end
    if(eventName == "point") then
        self:createEntity()
    end
end

function CircleOperations:CircleWith2Points(eventName, data)
    if (eventName == "point" and not Circle_FirstPoint) then
        Circle_FirstPoint = data['position']
    elseif (eventName == "mouseMove" and Circle_FirstPoint) then
        self.builder:setCenter(Circle_FirstPoint:mid(data["position"]))
        self.builder:setRadius(Circle_FirstPoint:distanceTo(data["position"]) / 2)
    elseif (eventName == "point" and Circle_FirstPoint) then
        self.builder:setCenter(Circle_FirstPoint:mid(data["position"]))
        self.builder:setRadius(Circle_FirstPoint:distanceTo(data["position"]) / 2)
        self:createEntity()
        Circle_FirstPoint = nil
    end
end

function CircleOperations:CircleWith3Points(eventName, data)
    if (eventName == "point" and not Circle_FirstPoint) then
        Circle_FirstPoint = data['position']
        message("Provide Second Point:",self.target_widget)
    elseif(eventName == "point" and Circle_FirstPoint and not Circle_SecondPoint) then
        message("Provide Third Point:",self.target_widget)
        Circle_SecondPoint = data['position']
    elseif(eventName == "point" and Circle_FirstPoint and Circle_SecondPoint and  not Circle_ThirdPoint) then
        Circle_ThirdPoint = data['position']
        self.builder:setCenter(self:Circumcenter(Circle_FirstPoint,Circle_SecondPoint,Circle_ThirdPoint))
        self.builder:setRadius(self.builder:center():distanceTo(Circle_ThirdPoint))
        self:createEntity()
    end
end

function CircleOperations:CircleWith3Tans(eventName, data)
    message("TODO:3 Tan Circle.",self.target_widget)
end

function CircleOperations:CircleWith2Tans(eventName, data)
    message("TODO:2 Tan Circle.",self.target_widget)
end

function CircleOperations:Circumcenter(Point1,Point2,Point3)
    local Angle1=Point1:angleBetween(Point2,Point3)
    local Angle2=Point2:angleBetween(Point3,Point1)
    local Angle3=Point3:angleBetween(Point1,Point2)
    local X = (Point1:x() * math.sin(2 * Angle1) + Point2:x() * math.sin(2 * Angle2) + Point3:x() * math.sin(2 * Angle3) ) / ( math.sin(2 * Angle1) + math.sin(2 * Angle2) + math.sin(2 * Angle3))
    local Y = (Point1:y() * math.sin(2 * Angle1) + Point2:y() * math.sin(2 * Angle2) + Point3:y() * math.sin(2 * Angle3) ) / ( math.sin(2 * Angle1) + math.sin(2 * Angle2) + math.sin(2 * Angle3))
    local Output=lc.geo.Coordinate(X,Y)
    return Output
end