DimOrdinateOperations = {
    name = "DimOrdinateOperations",
    command_line = "DIMORDINATE",
    icon = "dim_ordinate.svg",
    description = "Ordinate Dimension",
    menu_actions = {
        default = "actionOrdinate"
    }
}
DimOrdinateOperations.__index = DimOrdinateOperations

setmetatable(DimOrdinateOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimOrdinateOperations:_init()
    CreateOperations._init(self, lc.builder.DimOrdinateBuilder, "enterFeaturePoint")
    self.featurePoint = nil

    -- Ordinates are measured from the drawing's origin; LibreCAD has no UCS
    -- to move it.
    self.builder:setDefinitionPoint(lc.geo.Coordinate(0, 0))

    message("<b>Dimension Ordinate</b>")
    message("Click on the feature point")
end

-- Nothing to show until there is a feature to measure.
function DimOrdinateOperations:refreshTempEntity()
    if(self.featurePoint ~= nil) then
        CreateOperations.refreshTempEntity(self)
    end
end

function DimOrdinateOperations:enterFeaturePoint(eventName, data)
    if(eventName == "point") then
        self.featurePoint = data["position"]
        self.builder:dimAuto(self.featurePoint, self.featurePoint)
        self.step = "enterLeaderEnd"

        message("Options:<u>T</u>ext or Click on the end of the leader")
        mainWindow:cliCommand():returnText(true)
    end
end

-- Where the leader ends decides what is measured: sideways from the feature
-- measures Y, up or down measures X.
function DimOrdinateOperations:enterLeaderEnd(eventName, data)
    if(eventName == "text") then
        if (string.lower(data["text"]) == "t" or data["text"] == "text") then
            self.step = "enterText"
            message("Enter text override")
        else
            message("Invalid Option")
        end
    end

    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:dimAuto(self.featurePoint, data["position"])
    end

    if(eventName == "point") then
        mainWindow:cliCommand():returnText(false)
        self:createEntity()
    end
end

function DimOrdinateOperations:enterText(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:dimAuto(self.featurePoint, data["position"])
    end
    if(eventName == "text") then
        mainWindow:cliCommand():returnText(false)
        self.builder:setExplicitValue(data["text"])
        self.step = "enterLeaderEnd"
        message("Click on the end of the leader")
    end
end
