DimRadialOperations = {
    name = "DimRadialOperations",
    command_line = "DIMRADIAL",
    icon = "dim_radial.svg",
    menu_actions = {
        default = "actionRadius"
    }
}
DimRadialOperations.__index = DimRadialOperations

setmetatable(DimRadialOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimRadialOperations:_init()
    CreateOperations._init(self, lc.builder.DimRadialBuilder, "enterStartPoint")
    self.FirstPoint = nil
    message("<b>Dimension Radial</b>")
    message("Click on the center of curve or enter coordinates")
end

function DimRadialOperations:enterStartPoint(eventName, data)
    if(eventName == "point") then
        self.FirstPoint=data["position"]
        self.step = "enterEndPoint"
        message("Click point the curve or enter coordinates")
    end
end

function DimRadialOperations:enterEndPoint(eventName, data)
    if(eventName == "point") then
        self.builder:setMiddleOfText(data["position"])
        self.builder:setDefinitionPoint(self.FirstPoint)
        self.builder:setDefinitionPoint2(data["position"])
        self.step = "enterMiddleOfText"
        message("Options:<u>T</u>ext or Click on text position or enter coordinates")
        mainWindow:cliCommand():returnText( true)
    end
end

function DimRadialOperations:enterMiddleOfText(eventName, data)
    if(eventName == "text") then
        if (string.lower(data["text"]) == "t" or data["text"] == "text") then
            self.step = "enterText"
            message("Enter text override")
        else
            message("Invalid Option")
        end
    end

    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setMiddleOfText(data["position"])
    end

    if(eventName == "point") then
        self.ThirdPoint = data["position"]
        mainWindow:cliCommand():returnText( true)
        self:createEntity()
    end
end

function DimRadialOperations:enterText(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setMiddleOfText(data["position"])
    end
    if(eventName == "text") then
        mainWindow:cliCommand():returnText( false)
        self.builder:setExplicitValue(data["text"])
        self.step = "enterMiddleOfText"
        message("Click on text position")
    end
end