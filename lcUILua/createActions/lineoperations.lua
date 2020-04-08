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

function LineOperations:_init_rectangle()
    message("<b>LINE - RECTANGLE</b>", self.target_widget)
    message("Click on top left point of rectangle or enter coordinates:", self.target_widget)
    self.step = "RectangleWithCornerPoints"
end

function LineOperations:_init_polygon_cencor()
    message("<b>LINE - Polygon</b>", self.target_widget)
    message("Click on center of polygon or enter coordinates:", self.target_widget)
    self.step = "PolygonWithCenterPoint"
end

function LineOperations:_init_polygon_corcor()
    message("<b>LINE - Polygon</b>", self.target_widget)
    message("Click on first coordinate of polygon or enter coordinates:", self.target_widget)
    self.step = "PolygonWith2Points"
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

function LineOperations:RectangleWithCornerPoints(eventName, data)
    if(eventName == "point" and not self.topLeftPoint) then
        self.topLeftPoint = data["position"]
        message("Click on bottom right point or enter coordinates:-", self.target_widget)
    elseif(eventName == "mouseMove" and self.topLeftPoint) then
        if(self.tempEntities ~= nil) then
            for k, entity in pairs(self.tempEntities) do
                getWindow(self.target_widget):tempEntities():removeEntity(entity)
            end
        else
            self.tempEntities = {}
        end

        -- TOP LEFT -> TOP RIGHT
        self.builder:newID()
        self.builder:setStartPoint(self.topLeftPoint)
        self.builder:setEndPoint(lc.geo.Coordinate(data["position"]:x(), self.topLeftPoint:y()))
        self.tempEntities[0] = self.builder:build()
        getWindow(self.target_widget):tempEntities():addEntity(self.tempEntities[0])

        -- TOP RIGHT -> BOTTOM RIGHT
        self.builder:newID()
        self.builder:setStartPoint(lc.geo.Coordinate(data["position"]:x(), self.topLeftPoint:y()))
        self.builder:setEndPoint(data["position"])
        self.tempEntities[1] = self.builder:build()
        getWindow(self.target_widget):tempEntities():addEntity(self.tempEntities[1])

        -- BOTTOM RIGHT -> BOTTOM LEFT
        self.builder:newID()
        self.builder:setStartPoint(data["position"])
        self.builder:setEndPoint(lc.geo.Coordinate(self.topLeftPoint:x(), data["position"]:y()))
        self.tempEntities[2] = self.builder:build()
        getWindow(self.target_widget):tempEntities():addEntity(self.tempEntities[2])

        -- BOTTOM LEFT -> TOP LEFT
        self.builder:newID()
        self.builder:setStartPoint(lc.geo.Coordinate(self.topLeftPoint:x(), data["position"]:y()))
        self.builder:setEndPoint(self.topLeftPoint)
    elseif(eventName == "point" and self.topLeftPoint) then
        local b = lc.operation.EntityBuilder(getWindow(self.target_widget):document())
        if(self.tempEntities ~= nil) then
            for k, entity in pairs(self.tempEntities) do
                b:appendEntity(entity)
                getWindow(self.target_widget):tempEntities():removeEntity(entity)
            end
        end
        b:execute()
        self:createEntity()
        self:close()
    end
end

function LineOperations:PolygonWithCenterPoint(eventName, data)
    if(eventName == "point" and not self.polygonCenter) then
        self.polygonCenter = data["position"]
        self.tempEntities = {}
        message("Move the mouse to scroll through n-sides polygons,enter number of points or click to choose", self.target_widget)
    elseif(eventName == "mouseMove" and self.polygonCenter and not self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            getWindow(self.target_widget):tempEntities():removeEntity(entity)
        end
        -- 150 + () ensures that numPoints is atleast 3
        local numPoints = math.floor((150+self.polygonCenter:distanceTo(data["position"]))/50)
        local length = 50
        for i=0,numPoints-1 do
            local newCoord1 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos((3.14159265 * 2)/numPoints * i), length * math.sin((3.14159265 * 2)/numPoints * i)))
            local newCoord2 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos((3.14159265 * 2)/numPoints * (i+1)), length * math.sin((3.14159265 * 2)/numPoints * (i+1))))
            
            self.builder:newID()
            self.builder:setStartPoint(newCoord1)
            self.builder:setEndPoint(newCoord2)
            if i~=numPoints-1 then
                self.tempEntities[i] = self.builder:build()
                getWindow(self.target_widget):tempEntities():addEntity(self.tempEntities[i])
            end
        end
    elseif(eventName == "point" and self.polygonCenter and not self.numPoints) then
        self.numPoints = math.floor((150+self.polygonCenter:distanceTo(data["position"]))/50)
        message("Click on a polygon end point to finalize polygon", self.target_widget)
    elseif(eventName == "mouseMove" and self.polygonCenter and self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            getWindow(self.target_widget):tempEntities():removeEntity(entity)
        end
        local length = self.polygonCenter:distanceTo(data["position"])
        local angle = self.polygonCenter:angleTo(data["position"])
        for i=0,self.numPoints-1 do
            local newCoord1 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos(((3.14159265 * 2)/self.numPoints * i) + angle), length * math.sin(((3.14159265 * 2)/self.numPoints * i) + angle)))
            local newCoord2 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos(((3.14159265 * 2)/self.numPoints * (i+1)) + angle), length * math.sin(((3.14159265 * 2)/self.numPoints * (i+1) + angle))))
            
            self.builder:newID()
            self.builder:setStartPoint(newCoord1)
            self.builder:setEndPoint(newCoord2)
            if i~=self.numPoints-1 then
                self.tempEntities[i] = self.builder:build()
                getWindow(self.target_widget):tempEntities():addEntity(self.tempEntities[i])
            end
        end
    elseif(eventName == "point" and self.polygonCenter and self.numPoints) then
        local b = lc.operation.EntityBuilder(getWindow(self.target_widget):document())
        for k, entity in pairs(self.tempEntities) do
            getWindow(self.target_widget):tempEntities():removeEntity(entity)
        end
        local length = self.polygonCenter:distanceTo(data["position"])
        local angle = self.polygonCenter:angleTo(data["position"])
        for i=0,self.numPoints-1 do
            local newCoord1 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos(((3.14159265 * 2)/self.numPoints * i) + angle), length * math.sin(((3.14159265 * 2)/self.numPoints * i) + angle)))
            local newCoord2 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos(((3.14159265 * 2)/self.numPoints * (i+1)) + angle), length * math.sin(((3.14159265 * 2)/self.numPoints * (i+1) + angle))))
            
            self.builder:newID()
            self.builder:setStartPoint(newCoord1)
            self.builder:setEndPoint(newCoord2)
            if i~=self.numPoints-1 then
                self.tempEntities[i] = self.builder:build()
                b:appendEntity(self.tempEntities[i])
            end
        end
        b:execute()
        self:createEntity()
        self:close()
    end
end

function LineOperations:PolygonWith2Points(eventName, data)
    if(eventName == "point" and not self.firstPoint) then
        self.firstPoint = data["position"]
        self.tempEntities = {}
        message("Move the mouse to scroll through n-sides polygons,enter number of points or click to choose", self.target_widget)
    elseif(eventName == "mouseMove" and self.firstPoint and not self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            getWindow(self.target_widget):tempEntities():removeEntity(entity)
        end
        local length = 50
        self.polygonCenter = self.firstPoint:add(lc.geo.Coordinate(-length, 0))
        local numPoints = math.floor((150+self.polygonCenter:distanceTo(data["position"]))/50)
        for i=0,numPoints-1 do
            local newCoord1 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos((3.14159265 * 2)/numPoints * i), length * math.sin((3.14159265 * 2)/numPoints * i)))
            local newCoord2 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos((3.14159265 * 2)/numPoints * (i+1)), length * math.sin((3.14159265 * 2)/numPoints * (i+1))))
            
            self.builder:newID()
            self.builder:setStartPoint(newCoord1)
            self.builder:setEndPoint(newCoord2)
            if i~=numPoints-1 then
                self.tempEntities[i] = self.builder:build()
                getWindow(self.target_widget):tempEntities():addEntity(self.tempEntities[i])
            end
        end
    elseif(eventName == "point" and self.firstPoint and not self.numPoints) then
        self.numPoints = math.floor((150+self.polygonCenter:distanceTo(data["position"]))/50)
        message("Click on a polygon corner point to finalize polygon", self.target_widget)
    elseif(eventName == "mouseMove" and self.firstPoint and self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            getWindow(self.target_widget):tempEntities():removeEntity(entity)
        end

        local x = self.firstPoint:distanceTo(data["position"]) / 2
        local theta = (3.14159265/2) - (3.14159265/self.numPoints)
        local pointx = self.firstPoint:add(data["position"]):multiply(0.5)

        local y = x * math.tan(theta)
        local pointxnormal = pointx:sub(self.firstPoint):rotate(3.14159265/2):norm()
        self.polygonCenter = pointx:add(pointxnormal:multiply(y))

        local length = self.polygonCenter:distanceTo(data["position"])
        local angle = self.polygonCenter:angleTo(self.firstPoint)
        for i=0,self.numPoints-1 do
            local newCoord1 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos(((3.14159265 * 2)/self.numPoints * i) + angle), length * math.sin(((3.14159265 * 2)/self.numPoints * i) + angle)))
            local newCoord2 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos(((3.14159265 * 2)/self.numPoints * (i+1)) + angle), length * math.sin(((3.14159265 * 2)/self.numPoints * (i+1) + angle))))
            
            self.builder:newID()
            self.builder:setStartPoint(newCoord1)
            self.builder:setEndPoint(newCoord2)
            if i~=self.numPoints-1 then
                self.tempEntities[i] = self.builder:build()
                getWindow(self.target_widget):tempEntities():addEntity(self.tempEntities[i])
            end
        end
    elseif(eventName == "point" and self.firstPoint and self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            getWindow(self.target_widget):tempEntities():removeEntity(entity)
        end

        local x = self.firstPoint:distanceTo(data["position"]) / 2
        local theta = (3.14159265/2) - (3.14159265/self.numPoints)
        local pointx = self.firstPoint:add(data["position"]):multiply(0.5)

        local y = x * math.tan(theta)
        local pointxnormal = pointx:sub(self.firstPoint):rotate(3.14159265/2):norm()
        self.polygonCenter = pointx:add(pointxnormal:multiply(y))

        local length = self.polygonCenter:distanceTo(data["position"])
        local angle = self.polygonCenter:angleTo(self.firstPoint)
        local b = lc.operation.EntityBuilder(getWindow(self.target_widget):document())
        for i=0,self.numPoints-1 do
            local newCoord1 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos(((3.14159265 * 2)/self.numPoints * i) + angle), length * math.sin(((3.14159265 * 2)/self.numPoints * i) + angle)))
            local newCoord2 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos(((3.14159265 * 2)/self.numPoints * (i+1)) + angle), length * math.sin(((3.14159265 * 2)/self.numPoints * (i+1) + angle))))
            
            self.builder:newID()
            self.builder:setStartPoint(newCoord1)
            self.builder:setEndPoint(newCoord2)
            if i~=self.numPoints-1 then
                self.tempEntities[i] = self.builder:build()
                b:appendEntity(self.tempEntities[i])
            end
        end
        b:execute()
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

function LineOperations:close()
    if(self.tempEntities ~= nil) then
        for k, entity in pairs(self.tempEntities) do
            getWindow(self.target_widget):tempEntities():removeEntity(entity)
        end
    end
    CreateOperations.close(self)
end
