CopyOperation = {}
CopyOperation.__index = CopyOperation

setmetatable(CopyOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function CopyOperation:_init(id)
    Operations._init(self, id)

    self.selection = active_widget():selection()

    message(tostring(#self.selection) .. " items selected")

    if(#self.selection > 0) then
        self.origin = nil
        self.destination = nil

        self.tempEntities = {}

        luaInterface:registerEvent('point', self)
        luaInterface:registerEvent('mouseMove', self)

        message("Give origin point")
    else
        self.finished = true
        luaInterface:triggerEvent('operationFinished')
    end
end

function CopyOperation:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:tempCopy(...)
    end
end

function CopyOperation:newData(point)
    if(self.origin == nil) then
        self.origin = Operations:getCoordinate(point)

        message("Give destination point")
    elseif(Operations:getCoordinate(point) ~= nil) then
        self.destination = Operations:getCoordinate(point)

        self:copy()
    end
end

function CopyOperation:tempCopy(point)
    if(self.origin ~= nil) then
        for k, entity in pairs(self.tempEntities) do
            active_widget():tempEntities():removeEntity(entity)
        end
        self.tempEntities = {}

        local offset = point:sub(self.origin)

        for k, entity in pairs(self.selection) do
            local newEntity = entity:copy(offset)
            active_widget():tempEntities():addEntity(newEntity)
            table.insert(self.tempEntities, newEntity)
        end
    end
end

function CopyOperation:copy()
    local offset = self.destination:sub(self.origin)
    local b = EntityBuilder(active_widget():document())

    for k, entity in pairs(self.selection) do
        b:appendEntity(entity)
    end

    b:appendOperation(Push())
    b:appendOperation(Copy(offset))
    b:execute()

    self:close()
end

function CopyOperation:close()
    if(not self.finished) then
        self.finished = true

        for k, entity in pairs(self.tempEntities) do
            active_widget():tempEntities():removeEntity(entity)
        end

        luaInterface:deleteEvent('mouseMove', self)
        luaInterface:deleteEvent('point', self)

        luaInterface:triggerEvent('operationFinished')
    end
end