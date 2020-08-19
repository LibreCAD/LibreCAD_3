PointOperations = {
    name = "PointOperations",
    command_line = "POINT",
    icon = "point.svg",
    description = "Point",
    menu_actions = {
        default = "actionPoint"
    }
}
PointOperations.__index = PointOperations

setmetatable(PointOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function PointOperations:_init()
    CreateOperations._init(self, lc.builder.PointBuilder, "enterPoint")
    mainWindow:cliCommand():commandActive(true)
    message("Add a new point")
end

--function PointOperations:_init_default()
--    message("<b>POINT</b>")
--    message("Provide Point")
--    self.step = "enterPoint"
--end

function PointOperations:enterPoint(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:setCoordinate(data["position"])
    elseif (eventName == "point") then
        self.builder:setCoordinate(data["position"])
        self:createEntity()
    end
end
