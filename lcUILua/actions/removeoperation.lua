RemoveOperation = {}
RemoveOperation.__index = RemoveOperation

setmetatable(RemoveOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function RemoveOperation:_init(id)
    Operations._init(self, id)

    self.selection = active_widget():selection()

    self:remove()
end

function RemoveOperation:remove()
    local b = EntityBuilder(active_widget():document())

    for k, entity in pairs(self.selection) do
        b:appendEntity(entity)
    end

    b:appendOperation(Push())
    b:appendOperation(Remove())
    b:execute()

    message(tostring(#self.selection) .. " items removed")

    self:close()
end

function RemoveOperation:close()
    if(not self.finished) then
        self.finished = true
        luaInterface:triggerEvent('operationFinished')
    end
end