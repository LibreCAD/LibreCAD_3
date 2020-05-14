DimLinearOperations = {}
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

    message("Click on start point")
end

function DimLinearOperations:enterStartPoint(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint2(data["position"])
    end

    if(eventName == "point") then
        self.step = "enterEndPoint"

        message("Click on end point")
    end
end

function DimLinearOperations:enterEndPoint(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint3(data["position"])
    end

    if(eventName == "point") then
        self.step = "enterMiddleOfText"

        message("Click on text position")
    end
end

function DimLinearOperations:enterMiddleOfText(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint(data["position"])
        self.builder:setMiddleOfText(data["position"])
    end

    if(eventName == "point") then
        self.step = "enterText"

        mainWindow:getCliCommand():returnText( true)

        message("Enter dimension text (<> for value)")
    end
end

function DimLinearOperations:enterText(eventName, data)
    if(eventName == "text") then
        mainWindow:getCliCommand():returnText( false)
        self.builder:setExplicitValue(data["text"])
        self:createEntity()
    end
end
