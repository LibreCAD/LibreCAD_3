DimAlignedOperations = {
    name = "DimAlignedOperations",
    command_line = "DIMALIGNED",
    icon = "dim_aligned.svg",
    description = "Aligned Dimension",
    menu_actions = {
        default = "actionAligned"
    }
}
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
    if(eventName == "point") then
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
        message("Options:<u>T</u>ext or Click on text position")
        mainWindow:cliCommand():returnText( true)
    end
end

function DimAlignedOperations:enterMiddleOfText(eventName, data)
    if(eventName == "text") then
        if (string.lower(data["text"]) == "t" or data["text"] == "text") then
            self.step = "enterText"
            message("Enter text override")
        else
            message("Invalid Option")
        end
    end
    
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
        mainWindow:cliCommand():returnText( false)
        self:createEntity()
    end
end

function DimAlignedOperations:enterText(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:dimAuto(self.firstPoint,
                             self.secondPoint,
                             data["position"]
        )
    end
    if(eventName == "text") then
        mainWindow:cliCommand():returnText( false)
        self.builder:setExplicitValue(data["text"])
        self.step = "enterMiddleOfText"
        message("Click on text position")
    end
end
