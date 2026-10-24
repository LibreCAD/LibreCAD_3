DimLinearOperations = {
    name = "DimLinearOperations",
    command_line = "DIMLINEAR",
    icon = "dim_linear.svg",
    description = "Linear Dimension",
    menu_actions = {
        default = "actionLinear"
    }
}
DimLinearOperations.__index = DimLinearOperations

DimLinearOperations.HORIZONTAL = 0
DimLinearOperations.VERTICAL = math.pi / 2

setmetatable(DimLinearOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimLinearOperations:_init()
    CreateOperations._init(self, lc.builder.DimLinearBuilder, "enterStartPoint")
    self.FirstPoint = nil
    message("<b>Dimension Linear</b>")
    message("Click on start point")
end

-- Whether a linear dimension between first and second ({x, y} pairs) whose
-- line goes through cursor measures across (HORIZONTAL) or up (VERTICAL).
-- Above or below the box the two points span it measures across, left or right
-- of it up, and off a corner by whichever side the cursor is further past.
-- Inside the box the way the cursor moved ({dx, dy}) decides: moving clearly up
-- or down drags a horizontal dimension line, clearly sideways a vertical one.
-- Standing still or moving diagonally keeps current, so it does not flicker.
function DimLinearOperations.orientation(first, second, cursor, moved, current)
    local pastX = math.max(math.min(first[1], second[1]) - cursor[1], cursor[1] - math.max(first[1], second[1]), 0)
    local pastY = math.max(math.min(first[2], second[2]) - cursor[2], cursor[2] - math.max(first[2], second[2]), 0)
    if pastX > 0 or pastY > 0 then
        return pastY >= pastX and DimLinearOperations.HORIZONTAL or DimLinearOperations.VERTICAL
    end

    if math.abs(moved[2]) > 2 * math.abs(moved[1]) then
        return DimLinearOperations.HORIZONTAL
    elseif math.abs(moved[1]) > 2 * math.abs(moved[2]) then
        return DimLinearOperations.VERTICAL
    end
    return current
end

-- Put the dimension line through position, at the orientation the user locked
-- with H or V or else the one the cursor chooses.
function DimLinearOperations:place(position)
    local first, second = self.FirstPoint, self.SecondPoint
    if self.lockedAngle ~= nil then
        self.angle = self.lockedAngle
    else
        local moved = {0, 0}
        if self.lastPosition ~= nil then
            moved = {position:x() - self.lastPosition:x(), position:y() - self.lastPosition:y()}
        end
        self.angle = DimLinearOperations.orientation({first:x(), first:y()}, {second:x(), second:y()},
                                                     {position:x(), position:y()}, moved, self.angle)
    end
    self.lastPosition = position

    -- As DXF has it, the definition point is where the dimension line meets
    -- the second extension line.
    if self.angle == DimLinearOperations.HORIZONTAL then
        self.builder:setDefinitionPoint(lc.geo.Coordinate(second:x(), position:y()))
    else
        self.builder:setDefinitionPoint(lc.geo.Coordinate(position:x(), second:y()))
    end
    self.builder:setAngle(self.angle)
    self.builder:setMiddleOfText(position)
end

function DimLinearOperations:enterStartPoint(eventName, data)
    if(eventName == "point") then
        self.FirstPoint=data["position"]
        self.step = "enterEndPoint"

        message("Click on end point")
    end
end

function DimLinearOperations:enterEndPoint(eventName, data)
    if(eventName == "point") then
        self.SecondPoint = data["position"]
        self.builder:setMiddleOfText(data["position"])
        self.builder:setDefinitionPoint2(self.FirstPoint)
        self.builder:setDefinitionPoint3(data["position"])

        -- Until the cursor says otherwise, measure along the longer side.
        local dx = math.abs(self.SecondPoint:x() - self.FirstPoint:x())
        local dy = math.abs(self.SecondPoint:y() - self.FirstPoint:y())
        self.angle = dx >= dy and DimLinearOperations.HORIZONTAL or DimLinearOperations.VERTICAL

        self.step = "enterMiddleOfText"

        message("Options: <u>T</u>ext, <u>H</u>orizontal, <u>V</u>ertical or Click on text position")
        mainWindow:cliCommand():returnText( true)
    end
end

function DimLinearOperations:enterMiddleOfText(eventName, data)
    if(eventName == "text") then
        local option = string.lower(data["text"])
        if (option == "t" or option == "text") then
            self.step = "enterText"
            message("Enter text override")
        elseif (option == "h" or option == "horizontal" or option == "v" or option == "vertical") then
            self.lockedAngle = (option:sub(1, 1) == "h") and DimLinearOperations.HORIZONTAL or DimLinearOperations.VERTICAL
            if self.lastPosition ~= nil then
                self:place(self.lastPosition)
            end
            message("Click on text position")
        else
            message("Invalid Option")
        end
    end
    if(eventName == "mouseMove" or eventName == "point") then
        self:place(data["position"])
    end

    if(eventName == "point") then
        mainWindow:cliCommand():returnText( false)
        self:createEntity()
    end
end

function DimLinearOperations:enterText(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self:place(data["position"])
    end
    if(eventName == "text") then
        mainWindow:cliCommand():returnText( false)
        self.builder:setExplicitValue(data["text"])
        self.step = "enterMiddleOfText"
        message("Click on text position")
    end
end
