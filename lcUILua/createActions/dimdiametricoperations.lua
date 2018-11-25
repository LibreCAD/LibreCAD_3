DimDiametricOperations = {}
DimDiametricOperations.__index = DimDiametricOperations

setmetatable(DimDiametricOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimDiametricOperations:_init(id)
    CreateOperations._init(self, id, lc.builder.DimDiametricBuilder, "enterStartPoint")
    message("Click on start point", id)
end

function DimDiametricOperations:enterStartPoint(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint(data["position"])
    end

    if(eventName == "mouseMove") then
        self.builder:setDefinitionPoint2(data["position"]:add(lc.geo.Coordinate(10,0)))
    elseif(eventName == "point") then
        self.step = "enterEndPoint"

        message("Click on end point", self.target_widget)
    end
end

function DimDiametricOperations:enterEndPoint(eventName, data)
    if(eventName == "mouseMove" or eventName == "point") then
        self.builder:setDefinitionPoint2(data["position"])
    end

    if(eventName == "point") then
        self.step = "enterText"

        cli_get_text(self.target_widget, true)

        message("Enter dimension text (<> for value)", self.target_widget)
    end
end

function DimDiametricOperations:enterText(eventName, data)
    if(eventName == "text") then
        self.builder:setExplicitValue(data["text"])
        self:createEntity()
    end
end