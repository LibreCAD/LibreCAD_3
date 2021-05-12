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

function DimLinearOperations:enterStartPoint(eventName, data)
    if(eventName == "point") then
        self.FirstPoint=data["position"]
        self.step = "enterEndPoint"

        message("Click on end point")
    end
end

function DimLinearOperations:enterEndPoint(eventName, data)
    if(eventName == "point") then
        self.builder:setMiddleOfText(data["position"])
        self.builder:setDefinitionPoint2(self.FirstPoint)
        self.builder:setDefinitionPoint3(data["position"])

        self.step = "enterMiddleOfText"

        message("Options:<u>T</u>ext or Click on text position")
        mainWindow:cliCommand():returnText( true)
    end
end

function DimLinearOperations:enterMiddleOfText(eventName, data)
    if(eventName == "text") then
        if (string.lower(data["text"]) == "t" or data["text"] == "text") then
            self.step = "enterText"
            message("Enter text override")
        else
            message("Invalid Option")
        end
    end
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint(data["position"])
        self.builder:setMiddleOfText(data["position"])
    end

    if(eventName == "point") then
        mainWindow:cliCommand():returnText( false)
        self:createEntity()
    end
end

function DimLinearOperations:enterText(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint(data["position"])
        self.builder:setMiddleOfText(data["position"])
    end
    if(eventName == "text") then
        mainWindow:cliCommand():returnText( false)
        self.builder:setExplicitValue(data["text"])
        self.step = "enterMiddleOfText"
        message("Click on text position")
    end
end
