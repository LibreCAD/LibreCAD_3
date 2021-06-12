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
        message("Options:<u>T</u>ext or Click on opposite point on curve or enter coordinate")
        mainWindow:cliCommand():returnText( true)
    end
end

function DimDiametricOperations:enterEndPoint(eventName, data)
    if(eventName == "text") then
        if (string.lower(data["text"]) == "t" or data["text"] == "text") then
            self.step = "enterText"
            message("Enter text override")
        else
            message("Invalid Option")
        end
    end
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint2(data["position"])
    end

    if(eventName == "point") then
        self.builder:setDefinitionPoint2(data["position"])
        self:createEntity()
    end
end

function DimDiametricOperations:enterText(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:setDefinitionPoint2(data["position"])
    end
    if(eventName == "text") then
        mainWindow:cliCommand():returnText( false)
        self.builder:setExplicitValue(data["text"])
        self.step = "enterEndPoint"
        message("Click on opposite point on curve or enter coordinate")
    end
end
