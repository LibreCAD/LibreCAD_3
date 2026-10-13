DimAngularOperations = {
    name = "DimAngularOperations",
    command_line = "DIMANGULAR",
    icon = "dim_angular.svg",
    description = "Angular Dimension",
    menu_actions = {
        default = "actionANG3PT",
        twolines = "actionANG2LN"
    }
}
DimAngularOperations.__index = DimAngularOperations

setmetatable(DimAngularOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimAngularOperations:_init()
    CreateOperations._init(self, lc.builder.DimAngularBuilder, "enterCenterPoint")

    self.centerPoint = nil
    self.firstPoint = nil
end

function DimAngularOperations:_init_default()
    message("Click on center point")
end

function DimAngularOperations:_init_twolines()
    message("<b>Angular Dimension - Two Lines</b>")
    self.lines = {}

    -- Two lines already selected are the two to dimension.
    local selected = {}
    for _, entity in ipairs(mainWindow:cadMdiChild():selection()) do
        local line = lc.entity.Line.cast(entity)
        if line ~= nil then
            selected[#selected + 1] = line
        end
    end
    if #selected == 2 then
        self.lines = selected
        if self:setLines() then
            self.step = "placeArc"
            message("Click where the dimension arc goes")
            return
        end
        self.lines = {}
    end

    self.step = "pickLine"
    message("Click on the first line")
end

-- Where the line through a1 and a2 crosses the line through b1 and b2, all
-- {x, y} pairs, as x, y; nil when they are parallel.
function DimAngularOperations.lineIntersection(a1, a2, b1, b2)
    local ax, ay = a2[1] - a1[1], a2[2] - a1[2]
    local bx, by = b2[1] - b1[1], b2[2] - b1[2]
    local denominator = ax * by - ay * bx
    local scale = math.sqrt((ax * ax + ay * ay) * (bx * bx + by * by))
    if scale == 0 or math.abs(denominator) < 1e-9 * scale then
        return nil
    end
    local t = ((b1[1] - a1[1]) * by - (b1[2] - a1[2]) * bx) / denominator
    return a1[1] + t * ax, a1[2] + t * ay
end

-- The line of the drawing nearest to position, or nil when there is none.
function DimAngularOperations.nearestLine(position)
    local nearest, nearestDistance = nil, math.huge
    for _, entity in ipairs(mainWindow:cadMdiChild():document():entityContainer():asVector(32767)) do
        local line = lc.entity.Line.cast(entity)
        if line ~= nil then
            local distance = position:distanceTo(line:nearestPointOnEntity(position))
            if distance < nearestDistance then
                nearest, nearestDistance = line, distance
            end
        end
    end
    return nearest
end

-- Give the builder self.lines; false when they are parallel.
function DimAngularOperations:setLines()
    local function xy(p) return {p:x(), p:y()} end
    local first, second = self.lines[1], self.lines[2]
    local x, y = DimAngularOperations.lineIntersection(xy(first:start()), xy(first["end"](first)),
                                                       xy(second:start()), xy(second["end"](second)))
    if x == nil then
        return false
    end

    self.builder:setDefinitionPoint(lc.geo.Coordinate(x, y))
    self.builder:setDefLine11(first:start())
    self.builder:setDefLine12(first["end"](first))
    self.builder:setDefLine21(second:start())
    self.builder:setDefLine22(second["end"](second))
    return true
end

-- Two lines give nothing to preview until the arc has a point to go through.
function DimAngularOperations:refreshTempEntity()
    if self.lines == nil or self.arcThrough ~= nil then
        CreateOperations.refreshTempEntity(self)
    end
end

function DimAngularOperations:pickLine(eventName, data)
    if eventName ~= "point" then
        return
    end

    local line = DimAngularOperations.nearestLine(data["position"])
    if line == nil then
        message("There is no line to pick")
        return
    end
    if #self.lines == 1 and line:id() == self.lines[1]:id() then
        message("Click on a different line")
        return
    end

    self.lines[#self.lines + 1] = line
    if #self.lines == 1 then
        message("Click on the second line")
    elseif self:setLines() then
        self.step = "placeArc"
        message("Click where the dimension arc goes")
    else
        table.remove(self.lines)
        message("The lines are parallel: click on another second line")
    end
end

-- The arc goes through the point, in whichever of the four angles between
-- the lines the point is in.
function DimAngularOperations:placeArc(eventName, data)
    if eventName ~= "mouseMove" and eventName ~= "point" then
        return
    end
    -- At the vertex the arc would have no radius.
    if data["position"]:distanceTo(self.builder:definitionPoint()) < 1e-9 then
        return
    end

    self.arcThrough = data["position"]
    self.builder:setMiddleOfText(self.arcThrough)
    if eventName == "point" then
        self:createEntity()
    end
end

function DimAngularOperations:enterCenterPoint(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:dimAuto(data["position"],
                             data["position"]:add(lc.geo.Coordinate(10, 10)),
                             data["position"]:add(lc.geo.Coordinate(20, 10))
        )
    elseif(eventName == "point") then
        self.centerPoint = data["position"]
        self.step = "enterFirstPoint"

        message("Click on first point")
    end
end

function DimAngularOperations:enterFirstPoint(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:dimAuto(self.centerPoint,
                             data["position"],
                             data["position"]:add(lc.geo.Coordinate(10, 0))
        )
    elseif(eventName == "point") then
        self.firstPoint = data["position"]
        self.step = "enterSecondPoint"

        message("Click on second point")
    end
end

function DimAngularOperations:enterSecondPoint(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:dimAuto(self.centerPoint,
                             self.firstPoint,
                             data["position"]
        )
    elseif(eventName == "point") then
        self.secondPoint = data["position"]
        self.step = "enterText"

        mainWindow:cliCommand():returnText( true)

        message("Enter dimension text or leave it empty (<> for value)")
    end
end

function DimAngularOperations:enterText(eventName, data)
    if(eventName == "text") then
        mainWindow:cliCommand():returnText( false)
        self.builder:setExplicitValue(data["text"])
        self:createEntity()
    end
end
