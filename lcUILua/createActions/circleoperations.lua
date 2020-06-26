CircleOperations = {
    command_line = "CIRCLE",
    icon = "circle.svg",
    menu_actions = {
        cr = "actionCenter_Radius",
        cd = "actionCenter_Diameter",
        p2 = "action2_Point_Circle",
        p3 = "action3_Point_Circle_2",
        t2 = "actionTan_Tan_Radius",
        t3 = "actionTan_Tan_Tan"
    }
}
CircleOperations.__index = CircleOperations

setmetatable(CircleOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function CircleOperations:_init()
    CreateOperations._init(self, lc.builder.CircleBuilder, "enterCenter")
    mainWindow:cliCommand():commandActive(true)

    self.builder:setRadius(10)
    self.firstpoint = nil
    self.secondpoint = nil
    self.thirdpoint = nil
end

--test
function CircleOperations:_init_default()
	message("<b>CIRCLE</b>")
    message("Options: <b><u>2P</u>oint</b>, <u>3P</u>oint, <u>T</u>TT, TT<u>R</u>")
    message("Provide Center Point:")
	self.step = "CircleWithCenterRadius"
end

function CircleOperations:_init_cr()
    message("<b>CIRCLE</b>")
    message("Provide Center Point:")
    self.step = "CircleWithCenterRadius"
end

function CircleOperations:_init_cd()
    message("<b>CIRCLE</b>")
    message("Provide Center Point:")
    self.step = "CircleWithCenterDiameter"
end

function CircleOperations:_init_p2()
    message("<b>CIRCLE</b>")
    message("Provide First Point:")
    self.step = "CircleWith2Points"
end

function CircleOperations:_init_p3()
    message("<b>CIRCLE</b>")
    message("Provide First Point:") 
    self.step = "CircleWith3Points"
end

function CircleOperations:_init_t3()
    message("<b>CIRCLE</b>")
    self.step = "CircleWith3Tans"
end

function CircleOperations:_init_t2()
    message("<b>CIRCLE</b>")
    self.step = "CircleWith2Tans"
end

function CircleOperations:enterCenter(eventName, data)
    if(eventName == "point") then
        self.builder:setCenter(data["position"])
        message("Options: <u>R</u>adius , <u>D</u>iameter")
        message("Provide Radius:")
        self.step = "enterRadius"
    elseif(eventName == "text") then
        if ((data["text"]) == "2p" or string.lower(data["text"]) == "2point") then
            message("Provide Diameter Start Point:")
            self.step = "CircleWith2Points"
        elseif ((data["text"]) == "3p" or string.lower(data["text"]) == "3point") then
            message("Provide First Point:")
            self.step = "CircleWith3Points"
        elseif (string.lower(data["text"]) == "t" or string.lower(data["text"]) == "ttt") then
            message("Provide First Tangent:")
            self.step = "CircleWith3Tans"
        elseif (string.lower(data["text"]) == "r" or string.lower(data["text"]) == "ttr") then
            message("Provide First Tangent:")
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
                message("Provide Diameter:")
                self.step = "enterDiameter"
            else
                message("Invalid input:" .. data["text"] )
                message("Provide Radius:")
            end
        else
            message(data["text"])
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
            message("Invalid input:" .. data["text"] )
            message("Provide Diameter:")
        else
            message(data["text"])
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
        message("Provide Radius:")
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
        message("Provide Diameter:")
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
        message("Provide Second Point:")
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
        message("Provide Second Point:")
    elseif(eventName == "point" and self.firstpoint and not self.secondpoint) then
        message("Provide Third Point:")
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

-- Depending on position of mouse cursor, return appropriate s1,s2 and s3 for circle with 3 tans
function CircleOperations:getCircleWith3TansOptions(newPos)
    if(self.initialMousePosition == nil) then
        return 1,1,1
    end

    local base = lc.geo.Coordinate(self.initialMousePosition.x, 0)
    local angle = self.initialMousePosition:angleBetween(base, newPos)

    -- convert angle from range -pi to pi to 1 to 8
    angle = angle + 3.14159265
    angle = angle / (3.14159265 * 2)
    angle = angle * 8

    if(angle < 1) then
        return 1,1,1
    elseif(angle < 2) then
        return -1,1,1
    elseif(angle < 3) then
        return 1,-1,1
    elseif(angle < 4) then
        return 1,1,-1
    elseif(angle < 5) then
        return -1,-1,1
    elseif(angle < 6) then
        return 1,-1,-1
    elseif(angle < 7) then
        return -1,1,-1
    else
        return -1,-1,-1
    end
end

function CircleOperations:drawAllTan3Circles()
    self.builder:modifyForTempEntity(true)
    local oldId =self.builder:id()
    self.temp3tans = lc.operation.EntityBuilder(mainWindow:cadMdiChild():document())

    for i=-1,1 do
        for j=-1,1 do
            for k=-1,1 do
                if (i~=0 and j~=0 and k~=0) then
                    self.builder:newID()
                    self.builder:threeTanConstructor(self.selection[1], self.selection[2], self.selection[3], i, j, k)
                    self.temp3tans:appendEntity(self.builder:build())
                end
            end
        end
    end

    self.builder:setID(oldId)
    self.builder:modifyForTempEntity(false)

    self.temp3tans:execute()
end

function CircleOperations:getIndexForCircleWithTwoTan(newPos)
    local angle = self.twocirclecenters[1]:angleBetween(self.twocirclecenters[2], newPos)
    if(angle > 0) then
        return 0
    else
        return 1
    end
end

function CircleOperations:CircleWith3Tans(eventName, data)
    if(eventName == "mouseMove") then
        if(self.constructed3tan ~= true) then
            self.selection = mainWindow:cadMdiChild():selection()
            self.initialMousePosition = data["position"]
            local successful = false
            if(#self.selection == 3) then
                local success = self.builder:threeTanConstructor(self.selection[1], self.selection[2], self.selection[3], 1, 1, 1)
                if success == false then
                    message("Entities selected MUST be circles")
                    finish_operation()
                else
                    successful = true
                    self:refreshTempEntity()
                    self.constructed3tan = true
                    message("Move mouse around to cycle through different circles")
                end
            else
                message("THREE circle entities should be selected, there are " .. #self.selection .. " entities selected")
                finish_operation()
            end
            if (successful) then
                self:drawAllTan3Circles()
            end
        else
            local s1,s2,s3 = self:getCircleWith3TansOptions(data["position"])
            self.builder:threeTanConstructor(self.selection[1], self.selection[2], self.selection[3], s1, s2, s3)
            self:refreshTempEntity()
        end
    elseif(eventName == "point") then
        self:createEntity()
        self.temp3tans:appendOperation(lc.operation.Push())
        self.temp3tans:appendOperation(lc.operation.Remove())
        self.temp3tans:execute()
        self.constructed3tan = false
    end
end

function CircleOperations:CircleWith2Tans(eventName, data)
    if(eventName == "mouseMove") then
        if(self.constructed2tan ~= true) then
            self.selection = mainWindow:cadMdiChild():selection()
            self.initialMousePosition = data["position"]
            if(#self.selection == 2) then
                local success = self.builder:twoTanConstructor(self.selection[1], self.selection[2], -1, -1, 50, 0)
                if (success == -2) then
                    message("Entities selected MUST be circles")
                    finish_operation()
                elseif (success == -3) then
                    message("Circle cannot be created for selected circle entities (one circle should not be contained inside another)")
                    finish_operation()
                elseif (success == 0) then
                    self:refreshTempEntity()
                end
                message("Move mouse for radius, and around to cycle through different circles")
                -- get center of two circles from circlebuilder and set center between the two circles
                self.twocirclecenters = self.builder:twoTanCircleCenters()
                self.centerbetweentwocircles = self.twocirclecenters[1]:add(self.twocirclecenters[2])
                self.centerbetweentwocircles = self.centerbetweentwocircles:multiply(0.5)
                self.constructed2tan = true
            else
                message("TWO circle entities should be selected, there are " .. #self.selection .. " entities selected")
                finish_operation()
            end
        else
            local radius = self.centerbetweentwocircles:distanceTo(data["position"])
            local index = self:getIndexForCircleWithTwoTan(data["position"])
            self.builder:twoTanConstructor(self.selection[1], self.selection[2], -1, -1, radius, index)
            self:refreshTempEntity()
        end
    elseif(eventName == "point") then
        self:createEntity()
    end
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

function CircleOperations:cleanUp()
    if (self.constructed3tan) then
        self.temp3tans:appendOperation(lc.operation.Push())
        self.temp3tans:appendOperation(lc.operation.Remove())
        self.temp3tans:execute()
    end
end
