DimAngularOperations = {}
DimAngularOperations.__index = DimAngularOperations

setmetatable(DimAngularOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimAngularOperations:_init(id)
    CreateOperations._init(self, id, lc.builder.DimAngularBuilder, "enterCenterPoint")

    self.centerPoint = nil
    self.firstPoint = nil

    message("Click on center point", id)
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

        message("Click on first point", self.target_widget)
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

        message("Click on second point", self.target_widget)
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

        cli_get_text(self.target_widget, true)

        message("Enter dimension text or leave it empty (<> for value)", self.target_widget)
    end
end

function DimAngularOperations:enterText(eventName, data)
    if(eventName == "text") then
        cli_get_text(self.target_widget, false)
        self.builder:setExplicitValue(data["text"])
        self:createEntity()
    end
end