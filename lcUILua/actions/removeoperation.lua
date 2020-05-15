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

function init()

    mainWindow:getCliCommand:addCommand("REMOVE", function() run_basic_operation(RemoveOperation) end)
end

function RemoveOperation:_init(id)
    Operations._init(self)

    self.selection = mainWindow:getCadMdiChild():selection()

    self:remove()
end

function RemoveOperation:remove()
    local b = lc.operation.EntityBuilder(mainWindow:getCadMdiChild():document())

    for k, entity in pairs(self.selection) do
        b:appendEntity(entity)
    end

    b:appendOperation(lc.operation.Push())
    b:appendOperation(lc.operation.Remove())
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
