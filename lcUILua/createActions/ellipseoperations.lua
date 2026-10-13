EllipseOperations = {
    name = "EllipseOperations",
    command_line = {
        "ELLIPSE",
        ARCELLIPSE = "arc"
    },
    icon = "ellipse.svg",
    description = "Ellipse Operation",
    menu_actions = {
        default = "actionEllipse_Axis",
        arc = "actionEllipse_Arc",
        foci = "actionEllipse_FociPoints",
        fourpoints = "actionEllipse_4Points"
    }
}
EllipseOperations.__index = EllipseOperations

setmetatable(EllipseOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function EllipseOperations:_init()
    self.isArc = isArc or false
    self.Axis_StartPoint = nil
    self.Axis_EndPoint = nil
    self.Axis_CenterPoint = nil
    self.rotation = false
    CreateOperations._init(self, lc.builder.EllipseBuilder, "EllipsewithAxisEnd")
    mainWindow:cliCommand():returnText( true) -- This command prevents the user from entering coordinates in the command line. But at the same time it is needed for receiving text options from user. Alternate method need to be worked out.
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

function EllipseOperations:_init_fourpoints()
    message("<b>Ellipse</b>")
    message("Provide first point:")
    self.fourPoints = {}
    mainWindow:cliCommand():returnText(false) -- no text options here, so let the user type coordinates
    self.step = "EllipseFourPoints"
end

-- The ellipse with its axes parallel to X and Y through four points
-- ({x, y} pairs), as its center x, y and its semi-axes along X and along Y.
-- Nil when there is none, or when there is more than one, as there is
-- through the corners of a rectangle.
--
-- The ellipse is A u^2 + C v^2 + D u + E v = 1, in u, v measured from the
-- points' centroid and scaled to about 1.  The centroid of points on an
-- ellipse is inside it, so the ellipse cannot pass through u = v = 0, which
-- is what lets the right-hand side be 1.
function EllipseOperations.axisParallelEllipseThrough(points)
    local cx, cy = 0, 0
    for _, p in ipairs(points) do
        cx, cy = cx + p[1] / 4, cy + p[2] / 4
    end
    local scale = 0
    for _, p in ipairs(points) do
        scale = math.max(scale, math.abs(p[1] - cx), math.abs(p[2] - cy))
    end
    if scale == 0 then
        return nil
    end

    local m = {}
    for i, p in ipairs(points) do
        local u, v = (p[1] - cx) / scale, (p[2] - cy) / scale
        m[i] = {u * u, v * v, u, v, 1}
    end

    -- Gauss-Jordan elimination with partial pivoting.
    for col = 1, 4 do
        local pivot = col
        for row = col + 1, 4 do
            if math.abs(m[row][col]) > math.abs(m[pivot][col]) then
                pivot = row
            end
        end
        if math.abs(m[pivot][col]) < 1e-9 then
            return nil
        end
        m[col], m[pivot] = m[pivot], m[col]
        for row = 1, 4 do
            if row ~= col then
                local f = m[row][col] / m[col][col]
                for k = col, 5 do
                    m[row][k] = m[row][k] - f * m[col][k]
                end
            end
        end
    end
    local A, C = m[1][5] / m[1][1], m[2][5] / m[2][2]
    local D, E = m[3][5] / m[3][3], m[4][5] / m[4][4]
    if A <= 0 or C <= 0 then
        return nil
    end

    -- A (u - u0)^2 + C (v - v0)^2 = F
    local u0, v0 = -D / (2 * A), -E / (2 * C)
    local F = 1 + A * u0 * u0 + C * v0 * v0
    return cx + u0 * scale, cy + v0 * scale, math.sqrt(F / A) * scale, math.sqrt(F / C) * scale
end

function EllipseOperations:setFromFourPoints(points)
    local xy = {}
    for i, p in ipairs(points) do
        xy[i] = {p:x(), p:y()}
    end
    local x, y, alongX, alongY = EllipseOperations.axisParallelEllipseThrough(xy)
    if x == nil then
        return false
    end

    self.builder:setCenter(lc.geo.Coordinate(x, y))
    if alongX >= alongY then
        self.builder:setMajorPoint(lc.geo.Coordinate(alongX, 0))
        self.builder:setMinorRadius(alongY)
    else
        self.builder:setMajorPoint(lc.geo.Coordinate(0, alongY))
        self.builder:setMinorRadius(alongX)
    end
    return true
end

function EllipseOperations:EllipseFourPoints(eventName, data)
    if eventName ~= "point" and eventName ~= "mouseMove" then
        return
    end

    local points = {}
    for i, p in ipairs(self.fourPoints) do
        points[i] = p
    end
    points[#points + 1] = data["position"]

    if #points < 4 then
        if eventName == "point" then
            self.fourPoints = points
            message(({"Provide second point:", "Provide third point:", "Provide fourth point:"})[#points])
        end
    elseif self:setFromFourPoints(points) then
        if eventName == "point" then
            self:createEntity()
        end
    elseif eventName == "point" then
        message("No single ellipse with axes parallel to X and Y passes through these points")
        message("Provide another fourth point:")
    end
end

function EllipseOperations:EllipsewithAxisEnd(eventName, data)
    if(eventName == "text" and not self.Axis_StartPoint) then
        if (string.lower(data["text"]) == "c" or string.lower(data["text"]) == "center") then
            message("Provide Center Point:")
            mainWindow:cliCommand():returnText( false)
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
            mainWindow:cliCommand():returnText( false)
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
        mainWindow:cliCommand():returnText( true)
        message("Provide other axis end Point:")
    elseif (eventName == "text" and self.Axis_EndPoint) then
        if (string.lower(data["text"]) == "r" or string.lower(data["text"]) == "rotation") then
            message("Specify rotation:")
            self.rotation = true
            mainWindow:cliCommand():returnText( false)
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
        mainWindow:cliCommand():returnText( false)
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
