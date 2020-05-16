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

function LineOperations:init()
    mainWindow:connectMenuItem("action2_Point_Line", function() run_basic_operation(LineOperations, "_init_2p") end)
    mainWindow:connectMenuItem("actionPoint_Angle_Length_Line", function() run_basic_operation(LineOperations, "_init_pal") end)
    mainWindow:connectMenuItem("actionHorizontalLine", function() run_basic_operation(LineOperations, "_init_horizontal") end)
    mainWindow:connectMenuItem("actionVerticalLine", function() run_basic_operation(LineOperations, "_init_vertical") end)
    mainWindow:connectMenuItem("actionParallelLine", function() run_basic_operation(LineOperations, "_init_parallel") end)
    mainWindow:connectMenuItem("actionOrthogonalLine", function() run_basic_operation(LineOperations, "_init_orthogonal") end)
    mainWindow:connectMenuItem("actionRectangle", function() run_basic_operation(LineOperations, "_init_rectangle") end)
    mainWindow:connectMenuItem("actionPolygonCenCor", function() run_basic_operation(LineOperations, "_init_polygon_cencor") end)
    mainWindow:connectMenuItem("actionPolygonCenTan", function() run_basic_operation(LineOperations, "_init_polygon_centan") end)
    mainWindow:connectMenuItem("actionPolygonCorCor", function() run_basic_operation(LineOperations, "_init_polygon_corcor") end)

    mainWindow:getCliCommand():addCommand("LINE", function() run_basic_operation(LineOperations) end)
end

function LineOperations:_init()
    print("Called")
    CreateOperations._init(self, lc.builder.LineBuilder, "setFirstPoint")
    self.length = nil
end

function LineOperations:_init_default()
    message("<b>LINE</b>")
    --message("Options: <b><u>C</u>ontinuous</b>, <u>S</u>egment")  TODO: Multiple lines in single command.
    message("Click on first point or enter coordinates:")
    self.step = "setFirstPoint"
end

function LineOperations:_init_2p()
    message("<b>LINE</b>")
    --message("Options: <b><u>C</u>ontinuous</b>, <u>S</u>egment")  TODO: Multiple lines in single command.
    message("Click on first point or enter coordinates:")
    self.step = "setFirstPoint"
end

function LineOperations:_init_pal()
    message("<b>LINE</b>")
    --message("Options: <b><u>C</u>ontinuous</b>, <u>S</u>egment")  TODO: Multiple lines in single command.
    message("Click on first point or enter coordinates:")
    self.step = "LineWithPointAngleLength"
end

function LineOperations:_init_horizontal()
    message("<b>LINE - Horizontal Line</b>")
    message("Click on first point or enter coordinates:")
    self.step = "HorizontalLine"
end

function LineOperations:_init_vertical()
    message("<b>LINE - Vertical Line</b>")
    message("Click on first point or enter coordinates:")
    self.step = "VerticalLine"
end

function LineOperations:_init_parallel()
    message("<b>LINE - Line parallel to another line</b>")
    self.step = "LineParallelToLine"
end

function LineOperations:_init_orthogonal()
    message("<b>LINE - Line orthogonal to another line</b>")
    self.step = "LineOrthogonalToLine"
end

function LineOperations:_init_rectangle()
    message("<b>LINE - RECTANGLE</b>")
    message("Click on top left point of rectangle or enter coordinates:")
    self.step = "RectangleWithCornerPoints"
end

function LineOperations:_init_polygon_cencor()
    message("<b>LINE - Polygon</b>")
    message("Click on center of polygon or enter coordinates:")
    self.step = "PolygonWithCenterPoint"
end

function LineOperations:_init_polygon_centan()
    message("<b>LINE - Polygon</b>")
    message("Click on center of polygon or enter coordinates:")
    self.step = "PolygonWithCenterTangent"
end

function LineOperations:_init_polygon_corcor()
    message("<b>LINE - Polygon</b>")
    message("Click on first coordinate of polygon or enter coordinates:")
    self.step = "PolygonWith2Points"
end

function LineOperations:setFirstPoint(eventName, data)
    if(eventName == "point") then
        self.builder:setStartPoint(data["position"])
        message("Click on second point or enter coordinates or enter line length")
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
            message("Click on second point or enter coordinates or enter line angle")
        end
    end
end

function LineOperations:LineWithPointAngleLength(eventName, data)
    if(eventName == "point" and not self.startPoint) then
        self.builder:setStartPoint(data["position"])
        self.startPoint = data["position"]
        message("Click to select angle or enter angle (in degrees):")
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
        message("Enter length or click on point to at required length")
    elseif(eventName == "number" and self.startPoint and not self.angle) then
        self.angle = data["number"] * (3.14159265) / 180
        local p2 = lc.geo.Coordinate(50,0)
        p2 = p2:rotate(self.angle)
        self.builder:setEndPoint(self.startPoint:add(p2))
        message("Enter length or click on point to at required length")
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

function LineOperations:HorizontalLine(eventName, data)
    if(eventName == "mouseMove" and not self.firstPoint) then
        local distance = 100
        self.builder:setStartPoint(data["position"])
        local endpt = lc.geo.Coordinate(data["position"]:x() + distance, data["position"]:y())
        self.builder:setEndPoint(endpt)
    elseif(eventName == "point" and not self.firstPoint) then
        self.firstPoint = data["position"]
        message("Click to finalize distance for end point of the line or enter distance of end point from start point:")
    elseif(eventName == "mouseMove" and self.firstPoint) then
        local p2 = self.firstPoint:add(lc.geo.Coordinate(0,1))
        self.direction = 1
        local d = ((data["position"]:x() - self.firstPoint:x()) * (p2:y() - self.firstPoint:y())) - ((data["position"]:y() - self.firstPoint:y()) * (p2:x() - self.firstPoint:x()))
        if(d < 0) then
            self.direction = -1
        end
        local endpt = lc.geo.Coordinate(self.firstPoint:x() + self.direction * self.firstPoint:distanceTo(data["position"]), self.firstPoint:y())
        self.builder:setEndPoint(endpt)
    elseif(eventName == "number" and self.firstPoint) then
        local endpt = lc.geo.Coordinate(self.firstPoint:x() + self.direction * data["number"], self.firstPoint:y())
        self.builder:setEndPoint(endpt)
        self:createEntity()
        self:close()
    elseif(eventName == "point" and self.firstPoint) then
        local p2 = self.firstPoint:add(lc.geo.Coordinate(0,1))
        self.direction = 1
        local d = ((data["position"]:x() - self.firstPoint:x()) * (p2:y() - self.firstPoint:y())) - ((data["position"]:y() - self.firstPoint:y()) * (p2:x() - self.firstPoint:x()))
        if(d < 0) then
            self.direction = -1
        end
        local endpt = lc.geo.Coordinate(self.firstPoint:x() + self.direction * self.firstPoint:distanceTo(data["position"]), self.firstPoint:y())
        self.builder:setEndPoint(endpt)
        self:createEntity()
        self:close()
    end
end

function LineOperations:VerticalLine(eventName, data)
    if(eventName == "mouseMove" and not self.firstPoint) then
        local distance = 100
        self.builder:setStartPoint(data["position"])
        local endpt = lc.geo.Coordinate(data["position"]:x(), data["position"]:y() + distance)
        self.builder:setEndPoint(endpt)
    elseif(eventName == "point" and not self.firstPoint) then
        self.firstPoint = data["position"]
        message("Click to finalize distance for end point of the line or enter distance of end point from start point:")
    elseif(eventName == "mouseMove" and self.firstPoint) then
        local p2 = self.firstPoint:add(lc.geo.Coordinate(1,0))
        self.direction = -1
        local d = ((data["position"]:x() - self.firstPoint:x()) * (p2:y() - self.firstPoint:y())) - ((data["position"]:y() - self.firstPoint:y()) * (p2:x() - self.firstPoint:x()))
        if(d < 0) then
            self.direction = 1
        end
        local endpt = lc.geo.Coordinate(self.firstPoint:x(), self.firstPoint:y() + self.direction * self.firstPoint:distanceTo(data["position"]))
        self.builder:setEndPoint(endpt)
    elseif(eventName == "number" and self.firstPoint) then
        local endpt = lc.geo.Coordinate(self.firstPoint:x(), self.firstPoint:y() + self.direction * data["number"])
        self.builder:setEndPoint(endpt)
        self:createEntity()
        self:close()
    elseif(eventName == "point" and self.firstPoint) then
        local p2 = self.firstPoint:add(lc.geo.Coordinate(1,0))
        self.direction = -1
        local d = ((data["position"]:x() - self.firstPoint:x()) * (p2:y() - self.firstPoint:y())) - ((data["position"]:y() - self.firstPoint:y()) * (p2:x() - self.firstPoint:x()))
        if(d < 0) then
            self.direction = 1
        end
        local endpt = lc.geo.Coordinate(self.firstPoint:x(), self.firstPoint:y() + self.direction * self.firstPoint:distanceTo(data["position"]))
        self.builder:setEndPoint(endpt)
        self:createEntity()
        self:close()
    end
end

function LineOperations:LineParallelToLine(eventName, data)
    if(eventName == "mouseMove" and not self.firstPoint) then
        if(self.angle == nil) then
            self.selection = mainWindow:getCadMdiChild():selection()
            if(#self.selection == 1) then
                self.angle = self.builder:getLineAngle(self.selection[1])
                if(self.angle == -1) then
                    message("Entity selected should be a LINE")
                    finish_operation()
                else
                    message("Click on first point or enter coordinates:")
                end
            else
                message("Only one line entity should be selected, there are " .. #self.selection .. " entities selected")
                finish_operation()
            end
        end
        local start = data["position"]
        local endpt = lc.geo.Coordinate(100,0):rotate(self.angle)
        endpt = start:add(endpt)
        self.builder:setStartPoint(start)
        self.builder:setEndPoint(endpt)
    elseif(eventName == "point" and not self.firstPoint) then
        self.firstPoint = data["position"]
        message("Click to finalize distance for end point of the line or enter distance of end point from start point:")
    elseif(eventName == "mouseMove" and self.firstPoint) then
        -- determine direction of line end point based on mouse position
        local p2 = lc.geo.Coordinate(1,0):rotate(self.angle + 1.570796326)
        p2 = self.firstPoint:add(p2)
        self.direction = -1
        local d = ((data["position"]:x() - self.firstPoint:x()) * (p2:y() - self.firstPoint:y())) - ((data["position"]:y() - self.firstPoint:y()) * (p2:x() - self.firstPoint:x()))
        if(d < 0) then
            self.direction = 1
        end

        local endpt = lc.geo.Coordinate(data["position"]:distanceTo(self.firstPoint),0)
        if(self.direction < 0) then
            endpt = endpt:rotate(self.angle)
        else
            endpt = endpt:rotate(self.angle - 3.14159265)
        end
        endpt = self.firstPoint:add(endpt)
        self.builder:setStartPoint(self.firstPoint)
        self.builder:setEndPoint(endpt)
    elseif(eventName == "number" and self.firstPoint) then
        local endpt = lc.geo.Coordinate(data["number"],0)
        if(self.direction < 0) then
            endpt = endpt:rotate(self.angle)
        else
            endpt = endpt:rotate(self.angle - 3.14159265)
        end
        endpt = self.firstPoint:add(endpt)
        self.builder:setStartPoint(self.firstPoint)
        self.builder:setEndPoint(endpt)
        self:createEntity()
        self:close()
    elseif(eventName == "point" and self.firstPoint) then
        local endpt = lc.geo.Coordinate(data["position"]:distanceTo(self.firstPoint),0)
        if(self.direction < 0) then
            endpt = endpt:rotate(self.angle)
        else
            endpt = endpt:rotate(self.angle - 3.14159265)
        end
        endpt = self.firstPoint:add(endpt)
        self.builder:setStartPoint(self.firstPoint)
        self.builder:setEndPoint(endpt)
        self:createEntity()
        self:close()
    end
end

function LineOperations:LineOrthogonalToLine(eventName, data)
    if(eventName == "mouseMove" and not self.firstPoint) then
        if(self.angle == nil) then
            self.selection = mainWindow:getCadMdiChild():selection()
            if(#self.selection == 1) then
                self.angle = self.builder:getLineAngle(self.selection[1])
                if(self.angle == -1) then
                    message("Entity selected should be a LINE")
                    finish_operation()
                else
                    message("Click on first point or enter coordinates:")
                end
            else
                message("Only one line entity should be selected, there are " .. #self.selection .. " entities selected")
                finish_operation()
            end
        end
        local start = data["position"]
        local endpt = lc.geo.Coordinate(100,0):rotate(self.angle + 1.570796326)
        endpt = start:add(endpt)
        self.builder:setStartPoint(start)
        self.builder:setEndPoint(endpt)
    elseif(eventName == "point" and not self.firstPoint) then
        self.firstPoint = data["position"]
        message("Click to finalize distance for end point of the line or enter distance of end point from start point:")
    elseif(eventName == "mouseMove" and self.firstPoint) then
        -- determine direction of line end point based on mouse position
        local p2 = lc.geo.Coordinate(1,0):rotate(self.angle)
        p2 = self.firstPoint:add(p2)
        self.direction = -1
        local d = ((data["position"]:x() - self.firstPoint:x()) * (p2:y() - self.firstPoint:y())) - ((data["position"]:y() - self.firstPoint:y()) * (p2:x() - self.firstPoint:x()))
        if(d < 0) then
            self.direction = 1
        end

        local endpt = lc.geo.Coordinate(data["position"]:distanceTo(self.firstPoint),0):rotate(self.angle + self.direction * 1.570796326)
        endpt = self.firstPoint:add(endpt)
        self.builder:setStartPoint(self.firstPoint)
        self.builder:setEndPoint(endpt)
    elseif(eventName == "number" and self.firstPoint) then
        local endpt = lc.geo.Coordinate(data["number"],0):rotate(self.angle + 1.570796326)
        endpt = self.firstPoint:add(endpt)
        self.builder:setStartPoint(self.firstPoint)
        self.builder:setEndPoint(endpt)
        self:createEntity()
        self:close()
    elseif(eventName == "point" and self.firstPoint) then
        local endpt = lc.geo.Coordinate(data["position"]:distanceTo(self.firstPoint),0):rotate(self.angle + self.direction * 1.570796326)
        endpt = self.firstPoint:add(endpt)
        self.builder:setStartPoint(self.firstPoint)
        self.builder:setEndPoint(endpt)
        self:createEntity()
        self:close()
    end
end

function LineOperations:RectangleWithCornerPoints(eventName, data)
    if(eventName == "point" and not self.topLeftPoint) then
        self.topLeftPoint = data["position"]
        message("Click on bottom right point or enter coordinates:-")
    elseif(eventName == "mouseMove" and self.topLeftPoint) then
        if(self.tempEntities ~= nil) then
            for k, entity in pairs(self.tempEntities) do
                mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
            end
        else
            self.tempEntities = {}
        end

        -- TOP LEFT -> TOP RIGHT
        self.builder:newID()
        self.builder:setStartPoint(self.topLeftPoint)
        self.builder:setEndPoint(lc.geo.Coordinate(data["position"]:x(), self.topLeftPoint:y()))
        self.tempEntities[0] = self.builder:build()
        mainWindow:getCadMdiChild():tempEntities():addEntity(self.tempEntities[0])

        -- TOP RIGHT -> BOTTOM RIGHT
        self.builder:newID()
        self.builder:setStartPoint(lc.geo.Coordinate(data["position"]:x(), self.topLeftPoint:y()))
        self.builder:setEndPoint(data["position"])
        self.tempEntities[1] = self.builder:build()
        mainWindow:getCadMdiChild():tempEntities():addEntity(self.tempEntities[1])

        -- BOTTOM RIGHT -> BOTTOM LEFT
        self.builder:newID()
        self.builder:setStartPoint(data["position"])
        self.builder:setEndPoint(lc.geo.Coordinate(self.topLeftPoint:x(), data["position"]:y()))
        self.tempEntities[2] = self.builder:build()
        mainWindow:getCadMdiChild():tempEntities():addEntity(self.tempEntities[2])

        -- BOTTOM LEFT -> TOP LEFT
        self.builder:newID()
        self.builder:setStartPoint(lc.geo.Coordinate(self.topLeftPoint:x(), data["position"]:y()))
        self.builder:setEndPoint(self.topLeftPoint)
    elseif(eventName == "point" and self.topLeftPoint) then
        local b = lc.operation.EntityBuilder(mainWindow:getCadMdiChild():document())
        if(self.tempEntities ~= nil) then
            for k, entity in pairs(self.tempEntities) do
                b:appendEntity(entity)
                mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
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
        message("Move the mouse to scroll through n-sides polygons,enter number of points or click to choose")
    elseif(eventName == "mouseMove" and self.polygonCenter and not self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end
        -- 150 + () ensures that numPoints is atleast 3
        local numPoints = math.floor((150+self.polygonCenter:distanceTo(data["position"]))/50)
        local length = 50
        self:drawTempPolygon(length, 0, numPoints)
    elseif(eventName == "point" and self.polygonCenter and not self.numPoints) then
        self.numPoints = math.floor((150+self.polygonCenter:distanceTo(data["position"]))/50)
        message("Click on a polygon end point to finalize polygon")
    elseif(eventName == "mouseMove" and self.polygonCenter and self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end
        local length = self.polygonCenter:distanceTo(data["position"])
        local angle = self.polygonCenter:angleTo(data["position"])
        self:drawTempPolygon(length, angle, self.numPoints)
    elseif(eventName == "point" and self.polygonCenter and self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end
        local length = self.polygonCenter:distanceTo(data["position"])
        local angle = self.polygonCenter:angleTo(data["position"])
        self:drawFinalPolygon(length, angle)
        self:createEntity()
        self:close()
    end
end

function LineOperations:PolygonWith2Points(eventName, data)
    if(eventName == "point" and not self.firstPoint) then
        self.firstPoint = data["position"]
        self.tempEntities = {}
        message("Move the mouse to scroll through n-sides polygons,enter number of points or click to choose")
    elseif(eventName == "mouseMove" and self.firstPoint and not self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end
        local length = 50
        self.polygonCenter = self.firstPoint:add(lc.geo.Coordinate(-length, 0))
        local numPoints = math.floor((150+self.polygonCenter:distanceTo(data["position"]))/50)
        self:drawTempPolygon(length, 0, numPoints)
    elseif(eventName == "point" and self.firstPoint and not self.numPoints) then
        self.numPoints = math.floor((150+self.polygonCenter:distanceTo(data["position"]))/50)
        message("Click on a polygon corner point to finalize polygon")
    elseif(eventName == "mouseMove" and self.firstPoint and self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end

        local x = self.firstPoint:distanceTo(data["position"]) / 2
        local theta = (3.14159265/2) - (3.14159265/self.numPoints)
        local pointx = self.firstPoint:add(data["position"]):multiply(0.5)

        local y = x * math.tan(theta)
        local pointxnormal = pointx:sub(self.firstPoint):rotate(3.14159265/2):norm()
        self.polygonCenter = pointx:add(pointxnormal:multiply(y))

        local length = self.polygonCenter:distanceTo(data["position"])
        local angle = self.polygonCenter:angleTo(self.firstPoint)
        self:drawTempPolygon(length, angle, self.numPoints)
    elseif(eventName == "point" and self.firstPoint and self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end

        local x = self.firstPoint:distanceTo(data["position"]) / 2
        local theta = (3.14159265/2) - (3.14159265/self.numPoints)
        local pointx = self.firstPoint:add(data["position"]):multiply(0.5)

        local y = x * math.tan(theta)
        local pointxnormal = pointx:sub(self.firstPoint):rotate(3.14159265/2):norm()
        self.polygonCenter = pointx:add(pointxnormal:multiply(y))

        local length = self.polygonCenter:distanceTo(data["position"])
        local angle = self.polygonCenter:angleTo(self.firstPoint)
        self:drawFinalPolygon(length, angle)
        self:createEntity()
        self:close()
    end
end

function LineOperations:PolygonWithCenterTangent(eventName, data)
    if(eventName == "point" and not self.polygonCenter) then
        self.polygonCenter = data["position"]
        self.tempEntities = {}
        message("Move the mouse to scroll through n-sides polygons,enter number of points or click to choose")
    elseif(eventName == "mouseMove" and self.polygonCenter and not self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end
        local numPoints = math.floor((150+self.polygonCenter:distanceTo(data["position"]))/50)
        local length = 50
        self:drawTempPolygon(length, 0, numPoints)
    elseif(eventName == "point" and self.polygonCenter and not self.numPoints) then
        self.numPoints = math.floor((150+self.polygonCenter:distanceTo(data["position"]))/50)
        message("Click on a polygon tangent point to finalize polygon")
    elseif(eventName == "mouseMove" and self.polygonCenter and self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end

        local dist = self.polygonCenter:distanceTo(data["position"])
        local phi = (3.14159265/self.numPoints)
        local x = dist * math.tan(phi)

        local tangentvec = self.polygonCenter:sub(data["position"]):rotate(3.14159265/2):norm()
        self.firstPoint = data["position"]:add(tangentvec:multiply(x))

        local length = self.polygonCenter:distanceTo(self.firstPoint)
        local angle = self.polygonCenter:angleTo(self.firstPoint)
        self:drawTempPolygon(length, angle, self.numPoints)
    elseif(eventName == "point" and self.polygonCenter and self.numPoints) then
        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end

        local dist = self.polygonCenter:distanceTo(data["position"])
        local phi = (3.14159265/self.numPoints)
        local x = dist * math.tan(phi)

        local tangentvec = self.polygonCenter:sub(data["position"]):rotate(3.14159265/2):norm()
        self.firstPoint = data["position"]:add(tangentvec:multiply(x))

        local length = self.polygonCenter:distanceTo(self.firstPoint)
        local angle = self.polygonCenter:angleTo(self.firstPoint)
        self:drawFinalPolygon(length, angle)
        self:createEntity()
        self:close()
    end
end

function LineOperations:drawTempPolygon(length, angle, numPoints)
    for i=0,numPoints-1 do
        local newCoord1 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos(((3.14159265 * 2)/numPoints * i) + angle), length * math.sin(((3.14159265 * 2)/numPoints * i) + angle)))
        local newCoord2 = self.polygonCenter:add(lc.geo.Coordinate(length * math.cos(((3.14159265 * 2)/numPoints * (i+1)) + angle), length * math.sin(((3.14159265 * 2)/numPoints * (i+1) + angle))))
            
        self.builder:newID()
        self.builder:setStartPoint(newCoord1)
        self.builder:setEndPoint(newCoord2)
        if i~=numPoints-1 then
            self.tempEntities[i] = self.builder:build()
            mainWindow:getCadMdiChild():tempEntities():addEntity(self.tempEntities[i])
        end
    end
end

function LineOperations:drawFinalPolygon(length, angle)
    local b = lc.operation.EntityBuilder(mainWindow:getCadMdiChild():document())
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
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end
    end
    CreateOperations.close(self)
end
