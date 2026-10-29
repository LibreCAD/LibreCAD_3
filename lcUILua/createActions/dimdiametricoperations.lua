DimDiametricOperations = {
    name = "DimDiametricOperations",
    command_line = "DIMDIAMETRIC",
    icon = "dim_diametric.svg",
    description = "Diametric Dimension",
    menu_actions = {
        default = "actionDiameter"
    }
}
DimDiametricOperations.__index = DimDiametricOperations

setmetatable(DimDiametricOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimDiametricOperations:_init()
    CreateOperations._init(self, lc.builder.DimDiametricBuilder, "enterStartPoint")
    self.FirstPoint = nil
    message("<b>Dimension Diametric</b>")
    message("Click on the point on the curve or enter coordinates")
end

function DimDiametricOperations:enterStartPoint(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint(data["position"])
    end

    if(eventName == "mouseMove") then
        self.builder:setDefinitionPoint2(data["position"]:add(lc.geo.Coordinate(10,0)))
    elseif(eventName == "point") then
        self.step = "enterEndPoint"
        message("Click on opposite point on curve or enter coordinate")
        mainWindow:cliCommand():returnText(false)
    end
end

function DimDiametricOperations:enterEndPoint(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint2(data["position"])
    end

    if(eventName == "point") then
        self.builder:setDefinitionPoint2(data["position"])
        self.step = "enterText"
        mainWindow:cliCommand():returnText(true)
        message("Enter dimension text or leave empty for measured value")
    end
end

function DimDiametricOperations:enterText(eventName, data)
    if(eventName == "text") then
        mainWindow:cliCommand():returnText(false)
        self.builder:setExplicitValue(data["text"])
        self:createEntity()
    end
end
