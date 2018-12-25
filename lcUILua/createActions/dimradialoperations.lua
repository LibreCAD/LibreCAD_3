DimRadialOperations = {}
DimRadialOperations.__index = DimRadialOperations

setmetatable(DimRadialOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimRadialOperations:_init(id)
    CreateOperations._init(self, id, lc.builder.DimRadialBuilder, "enterStartPoint")

    message("Click on first definition point", id)
end

function DimRadialOperations:enterStartPoint(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint(data["position"])
    end

    if(eventName == "point") then
        self.step = "enterEndPoint"

        message("Click on end point", self.target_widget)
    end
end

function DimRadialOperations:enterEndPoint(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint2(data["position"])
    end

    if(eventName == "point") then
        self.step = "enterMiddleOfText"

        message("Click on text position", self.target_widget)
    end
end

function DimRadialOperations:enterMiddleOfText(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setMiddleOfText(data["position"])
    end

    if(eventName == "point") then
        self.step = "enterText"

        cli_get_text(self.target_widget, true)

        message("Enter dimension text (<> for value)", self.target_widget)
    end
end

function DimRadialOperations:enterText(eventName, data)
    if(eventName == "text") then
        cli_get_text(self.target_widget, false)
        self.builder:setExplicitValue(data["text"])
        self:createEntity()
    end
end