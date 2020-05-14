DimAlignedOperations = {}
DimAlignedOperations.__index = DimAlignedOperations

setmetatable(DimAlignedOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimAlignedOperations:_init()
    CreateOperations._init(self, lc.builder.DimAlignedBuilder, "enterFirstPoint")

    self.firstPoint = nil
    self.secondPoint = nil

    message("Click on start point", id)
end

function DimAlignedOperations:enterFirstPoint(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:dimAuto(data["position"],
                             data["position"]:add(lc.geo.Coordinate(10, 0)),
                             data["position"]:add(lc.geo.Coordinate(0, 10))
        )
    elseif(eventName == "point") then
        self.firstPoint = data["position"]
        self.step = "enterSecondPoint"

        message("Click on end point")
    end
end

function DimAlignedOperations:enterSecondPoint(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:dimAuto(self.firstPoint,
                             data["position"],
                             data["position"]:add(lc.geo.Coordinate(0, 10))
        )
    elseif(eventName == "point") then
        self.secondPoint = data["position"]
        self.step = "enterMiddleOfText"

        message("Click on text position")
    end
end

function DimAlignedOperations:enterMiddleOfText(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:dimAuto(self.firstPoint,
                             self.secondPoint,
                             data["position"]
        )
    elseif(eventName == "point") then
        self.builder:dimAuto(self.firstPoint,
                             self.secondPoint,
                             data["position"]
        )
        self.step = "enterText"
        message("Enter text (<> for value)")
        mainWindow:getCliCommand():returnText( true)
    end
end

function DimAlignedOperations:enterText(eventName, data)
    if(eventName == "text") then
        mainWindow:getCliCommand():returnText( false)
        self.builder:setExplicitValue(data["text"])
        self:createEntity()
    end
end
