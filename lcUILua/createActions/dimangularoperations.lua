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

function DimAngularOperations:_init()
    CreateOperations._init(self, lc.builder.DimAngularBuilder, "enterCenterPoint")

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

        message("Click on first point")
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

        message("Click on second point")
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

        mainWindow:getCliCommand():returnText( true)

        message("Enter dimension text or leave it empty (<> for value)")
    end
end

function DimAngularOperations:enterText(eventName, data)
    if(eventName == "text") then
        mainWindow:getCliCommand():returnText( false)
        self.builder:setExplicitValue(data["text"])
        self:createEntity()
    end
end
