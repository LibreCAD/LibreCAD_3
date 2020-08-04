HatchOperation = {
    command_line = "HATCH",
    icon = "hatch.svg"
}
HatchOperation.__index = HatchOperation

setmetatable(HatchOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function HatchOperation:_init(id)
    Operations._init(self)

    self.selection = mainWindow:cadMdiChild():selection()

    message(tostring(#self.selection) .. " items selected")

    if(#self.selection > 0) then
        self.origin = nil
        self.tempEntities = {}

        luaInterface:registerEvent('point', self)
        luaInterface:registerEvent('mouseMove', self)

        message("Give origin point")
    else
        self.finished = true
        luaInterface:triggerEvent('operationFinished')
    end
end

function HatchOperation:onEvent(eventName, event)
    if(eventName == "point" or eventName == "number") then
        --self:newData(event["position"])
    elseif(eventName == "mouseMove") then
        --self:tempMove(event["position"])
    end
end

function HatchOperation:close()
    if(not self.finished) then
        local window = mainWindow:cadMdiChild()
        self.finished = true

        luaInterface:deleteEvent('mouseMove', self)
        luaInterface:deleteEvent('point', self)

        luaInterface:triggerEvent('operationFinished')
    end
end
