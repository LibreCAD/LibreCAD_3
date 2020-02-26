CreateOperations = {}

setmetatable(CreateOperations, {
    __index = Operations
})

function CreateOperations:_init(id, builder, step)
    Operations._init(self, id)

    self.prevEntity = nil
    if(builder ~= nil) then
        self.builder = builder() -- is it working?
    end

    self.step = step

    self:registerEvents()
end

function CreateOperations:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(self.step ~= nil) then
        self[self.step](self, eventName, data)
    end

    if(not self.finished) then
        self:refreshTempEntity()
    end
end

function CreateOperations:createEntity()
    local b = lc.operation.EntityBuilder(getWindow(self.target_widget):document())
    b:appendEntity(self:build())
    b:execute()

    self:manualClose()
end

function CreateOperations:build()
    self.builder:setLayer(active_layer(self.target_widget))
    self.builder:setMetaInfo(active_metaInfo(self.target_widget))
    self.builder:setBlock(active_block(self.target_widget))

    return self.builder:build()
end

function CreateOperations:refreshTempEntity()
    if (self.prevEntity ~= nil) then
        getWindow(self.target_widget):tempEntities():removeEntity(self.prevEntity)
    end

    self.entity = self:build()

    if (self.entity ~= nil) then
        getWindow(self.target_widget):tempEntities():addEntity(self.entity)
    end

    self.prevEntity = self.entity
end

function CreateOperations:removeTempEntity()
    if (self.prevEntity ~= nil) then
        getWindow(self.target_widget):tempEntities():removeEntity(self.prevEntity)
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
        luaInterface:triggerEvent('operationFinished', self.target_widget)
        self:removeTempEntity()
        if (self.cleanUp ~= nil) then
            self:cleanUp()
        end
        self:unregisterEvents()
        cli_get_text(self.target_widget, false)
        cli_command_active(self.target_widget, false)
        self.finished = true
    end
end

function CreateOperations:manualClose()
    self:close() --if not defined manually
end
