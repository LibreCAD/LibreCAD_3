MoveOperation = {}
MoveOperation.__index = MoveOperation

setmetatable(MoveOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function init()

    mainWindow:getCliCommand:addCommand("MOVE", function() run_basic_operation(MoveOperation) end)
end

function MoveOperation:_init(id)
    Operations._init(self)

    self.selection = mainWindow:getCadMdiChild():selection()

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

function MoveOperation:onEvent(eventName, event)
    if(eventName == "point" or eventName == "number") then
        self:newData(event["position"])
    elseif(eventName == "mouseMove") then
        self:tempMove(event["position"])
    end
end

function MoveOperation:newData(point)
    if(self.origin == nil) then
        self.origin = Operations:getCoordinate(point)

        message("Give destination point")
    elseif(Operations:getCoordinate(point) ~= nil) then
        local offset = point:sub(self.origin)
        local b = lc.operation.EntityBuilder(mainWindow:getCadMdiChild():document())

        for k, entity in pairs(self.selection) do
            b:appendEntity(entity)
        end

        b:appendOperation(lc.operation.Push())
        b:appendOperation(lc.operation.Move(offset))
        b:execute()

        self:close()
    end
end

function MoveOperation:tempMove(point)
    if(self.origin ~= nil) then
        local window = mainWindow:getCadMdiChild()
        for k, entity in pairs(self.tempEntities) do
            window:tempEntities():removeEntity(entity)
        end
        self.tempEntities = {}

        local offset = point:sub(self.origin)

        for k, entity in pairs(self.selection) do
            local newEntity = entity:move(offset)
            window:tempEntities():addEntity(newEntity)
            table.insert(self.tempEntities, newEntity)
        end
    end
end

function MoveOperation:close()
    if(not self.finished) then
        local window = mainWindow:getCadMdiChild()
        self.finished = true

        for k, entity in pairs(self.tempEntities) do
            window:tempEntities():removeEntity(entity)
        end

        luaInterface:deleteEvent('mouseMove', self)
        luaInterface:deleteEvent('point', self)

        luaInterface:triggerEvent('operationFinished')
    end
end
