CreateOperations = {}

setmetatable(CreateOperations, {
    __index = Operations
})

function CreateOperations:_init(id)
    Operations._init(self, id)

    self.prevEntity = nil
    self:refreshTempEntity()
    self:registerEvents()
end

function CreateOperations:createEntity(entity)
    local b = EntityBuilder(active_widget():document())
    b:appendEntity(entity)
    b:execute()
end

function CreateOperations:refreshTempEntity()

    if (self.prevEntity ~= nil) then
        active_widget():tempEntities():removeEntity(self.prevEntity)
    end

    if (self.entity ~= nil) then
        active_widget():tempEntities():addEntity(self.entity)
    end

    self.prevEntity = self.entity
end

function CreateOperations:removeTempEntity()
    if (self.prevEntity ~= nil) then
        active_widget():tempEntities():removeEntity(self.prevEntity)
    end
end

function CreateOperations:unregisterEvents()
    luaInterface:deleteEvent('mouseMove', self)
    luaInterface:deleteEvent('number', self)
    luaInterface:deleteEvent('point', self)
    luaInterface:deleteEvent('text', self)
end

function CreateOperations:registerEvents()
    luaInterface:registerEvent('point', self)
    luaInterface:registerEvent('mouseMove', self)
    luaInterface:registerEvent('number', self)
    luaInterface:registerEvent('text', self)
end

function CreateOperations:close()
    if(not self.finished) then
        luaInterface:triggerEvent('operationFinished')
        self:removeTempEntity()
        self:unregisterEvents()
        self.finished = true
    end
end