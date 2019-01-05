CircleOperations = {}
CircleOperations.__index = CircleOperations

setmetatable(CircleOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        CreateOperations._init(self, lc.builder.CircleBuilder, ...)
        return self
    end,
})

function CircleOperations:_init_default()
    message("<b>Default circle constructor</b>", self.target_widget)
    self.step = "enterCenter"
    cli_get_text(self.target_widget, true) -- This command prevents the user from entering coordinates in commandline. But at the same time it is needed for receiving text options from user. Alternate method need to be workedout.
    message("<b>CIRCLE</b>", self.target_widget)
    message("Options: <u>2</u>P, <u>3</u>P, <u>T</u>TT, TT<u>R</u>", self.target_widget)
    message("Provide Center Point:", self.target_widget)
end

function CircleOperations:_init_cr()
    message("<b>CIRCLE</b>", self.target_widget)
    message("Provide Center Point:", self.target_widget)
    self.step = "CircleWithCenterRadius"
end

function CircleOperations:_init_cd()
    message("<b>CIRCLE</b>", self.target_widget)
    message("Provide Center Point:", self.target_widget)
    self.step = "CircleWithCenterDiameter"
end

function CircleOperations:_init_2p()
    message("<b>CIRCLE</b>", self.target_widget)
    message("Provide First Point:", self.target_widget)
    self.step = "CircleWith2Points"
end

function CircleOperations:_init_3p()
    message("<b>CIRCLE</b>", self.target_widget)
    message("Provide First Point:", self.target_widget) 
    self.step = "CircleWith3Points"
end

function CircleOperations:_init_3t()
    message("<b>CIRCLE</b>", self.target_widget)
    self.step = "CircleWith3Tans"
end

function CircleOperations:_init_2t()
    message("<b>CIRCLE</b>", self.target_widget)
    self.step = "CircleWith2Tans"
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

function CircleOperations:CircleWithCenterRadius(eventName, data)
    if (eventName == "point" and not self.centerPoint) then
        self.centerPoint=data["position"]
        self.builder:setCenter(data["position"])
        message("Provide Radius:", self.target_widget)
    elseif (eventName == "mouseMove" and self.centerPoint) then
        self.builder:setRadius(self.builder:center():distanceTo(data["position"]))
    elseif (eventName == "point" and self.centerPoint) then
        self.builder:setRadius(self.builder:center():distanceTo(data["position"]))
        self:createEntity()
    elseif (eventName == "number" and self.centerPoint) then
        self.builder:setRadius(data["number"])
        self:createEntity()
    end
end

function CircleOperations:CircleWithCenterDiameter(eventName, data)
    if (eventName == "point" and not self.centerPoint) then
        self.centerPoint=data["position"]
        self.builder:setCenter(data["position"])
        message("Provide Diameter:", self.target_widget)
    elseif (eventName == "mouseMove" and self.centerPoint) then
        self.builder:setRadius(self.builder:center():distanceTo(data["position"])/2)
    elseif (eventName == "point" and self.centerPoint) then
        self.builder:setRadius(self.builder:center():distanceTo(data["position"])/2)
        self:createEntity()
    elseif (eventName == "number" and self.centerPoint) then
        self.builder:setRadius(data["number"]/2)
        self:createEntity()
    end
end

function CircleOperations:CircleWith2Points(eventName, data)
    if (eventName == "point" and not self.firstpoint) then
        self.firstpoint = data['position']
        message("Provide Second Point:", self.target_widget)
    elseif (eventName == "mouseMove" and self.firstpoint) then
        self.builder:setCenter(self.firstpoint:mid(data["position"]))
        self.builder:setRadius(self.firstpoint:distanceTo(data["position"]) / 2)
    elseif (eventName == "point" and self.firstpoint) then
        self.builder:setCenter(self.firstpoint:mid(data["position"]))
        self.builder:setRadius(self.firstpoint:distanceTo(data["position"]) / 2)
        self:createEntity()
    end
end

function CircleOperations:CircleWith3Points(eventName, data)
    if (eventName == "point" and not self.firstpoint) then
        self.firstpoint = data['position']
        message("Provide Second Point:",self.target_widget)
    elseif(eventName == "point" and self.firstpoint and not self.secondpoint) then
        message("Provide Third Point:",self.target_widget)
        self.secondpoint = data['position']
    elseif(eventName == "mouseMove" and self.firstpoint and self.secondpoint and  not self.thirdpoint) then
        self.builder:setCenter(self:Circumcenter(self.firstpoint,self.secondpoint,data['position']))
        self.builder:setRadius(self.builder:center():distanceTo(data['position']))
    elseif(eventName == "point" and self.firstpoint and self.secondpoint and  not self.thirdpoint) then
        self.thirdpoint = data['position']
        self.builder:setCenter(self:Circumcenter(self.firstpoint,self.secondpoint,self.thirdpoint))
        self.builder:setRadius(self.builder:center():distanceTo(self.thirdpoint))
        self:createEntity()
    end
end

function CircleOperations:CircleWith3Tans(eventName, data)
    message("TODO:3 Tan Circle.",self.target_widget) -- This function requires picking or selecting CIRCLE or ARC entities. Once picking or selcting of objects starts working this function can be coded.
    finish_operation(self.target_widget)
end

function CircleOperations:CircleWith2Tans(eventName, data)
    message("TODO:2 Tan Circle.",self.target_widget) -- This function requires picking or selecting CIRCLE or ARC entities. Once picking or selcting of objects starts working this function can be coded.
    finish_operation(self.target_widget)
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