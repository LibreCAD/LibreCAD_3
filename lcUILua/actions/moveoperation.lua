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

function MoveOperation:_init(id)
    Operations._init(self, id)

    self.selection = active_widget():selection()

    message(tostring(#self.selection) .. " items selected")

    if(#self.selection > 0) then
        self.origin = nil
        self.tempEntities = {}

        event.register('point', self)
        event.register('mouseMove', self)

        message("Give origin point")
    else
        self.finished = true
        event.trigger('operationFinished')
    end
end

function MoveOperation:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:tempMove(...)
    end
end

function MoveOperation:newData(point)
    if(self.origin == nil) then
        self.origin = Operations:getCoordinate(point)

        message("Give destination point")
    elseif(Operations:getCoordinate(point) ~= nil) then
        local offset = point:sub(self.origin)
        local b = EntityBuilder(active_widget():document())

        for k, entity in pairs(self.selection) do
            b:appendEntity(entity)
        end

        b:appendOperation(Push())
        b:appendOperation(Move(offset))
        b:execute()

        self:close()
    end
end

function MoveOperation:tempMove(point)
    if(self.origin ~= nil) then
        for k, entity in pairs(self.tempEntities) do
            active_widget():tempEntities():removeEntity(entity)
        end
        self.tempEntities = {}

        local offset = point:sub(self.origin)

        for k, entity in pairs(self.selection) do
            local newEntity = entity:move(offset)
            active_widget():tempEntities():addEntity(newEntity)
            table.insert(self.tempEntities, newEntity)
        end
    end
end

function MoveOperation:close()
    if(not self.finished) then
        self.finished = true

        for k, entity in pairs(self.tempEntities) do
            active_widget():tempEntities():removeEntity(entity)
        end

        event.delete('mouseMove', self)
        event.delete('point', self)

        event.trigger('operationFinished')
    end
end