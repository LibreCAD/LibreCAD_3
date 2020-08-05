AreaOperation = {
    command_line = "AREA",
    icon = "infoarea.png"
}
AreaOperation.__index = AreaOperation

setmetatable(AreaOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function AreaOperation:_init(id)
    Operations._init(self)

    self.selection = mainWindow:cadMdiChild():selection()

    message(tostring(#self.selection) .. " items selected")

    if(#self.selection > 0) then
        self.origin = nil
        local reg = lc.geo.Region(self.selection)
        local area = reg:Area()
        message('Area\t: ' .. tostring(area))
        message('Error ~= 0.01 %')--i think
    else
        self.finished = true
        luaInterface:triggerEvent('operationFinished')
    end
end

function AreaOperation:close()
    if(not self.finished) then
        local window = mainWindow:cadMdiChild()
        self.finished = true
        luaInterface:triggerEvent('operationFinished')
    end
end
