CreateOperations = {}

setmetatable(CreateOperations, {
    __index = Operations
})

function CreateOperations:_init(builder, step)
    Operations._init(self)

    self.prevEntity = nil
    if(builder ~= nil) then
        self.builder = builder() -- is it working?
    end

    self.step = step

    print("REGISTER EVENTS?")
    self:registerEvents()
end

function CreateOperations:onEvent(eventName, data)
    if(self.step ~= nil) then
        self[self.step](self, eventName, data)
    end

    if(not self.finished) then
        self:refreshTempEntity()
    end
end

function CreateOperations:createEntity()
    local b = lc.operation.EntityBuilder(mainWindow:getCadMdiChild():document())
    b:appendEntity(self:build())
    b:execute()

    self:manualClose()
end

function CreateOperations:build()
    self.builder:setLayer(mainWindow:getCadMdiChild():activeLayer())
    self.builder:setMetaInfo(mainWindow:getCadMdiChild():metaInfoManager():metaInfo())
    self.builder:setBlock(mainWindow:getCadMdiChild():activeViewport())

    return self.builder:build()
end

function CreateOperations:refreshTempEntity()
    if (self.prevEntity ~= nil) then
        mainWindow:getCadMdiChild():tempEntities():removeEntity(self.prevEntity)
    end

    self.entity = self:build()

    if (self.entity ~= nil) then
        mainWindow:getCadMdiChild():tempEntities():addEntity(self.entity)
    end

    self.prevEntity = self.entity
end

function CreateOperations:removeTempEntity()
    if (self.prevEntity ~= nil) then
        mainWindow:getCadMdiChild():tempEntities():removeEntity(self.prevEntity)
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
        if (self.cleanUp ~= nil) then
            self:cleanUp()
        end
        self:unregisterEvents()
        mainWindow:getCliCommand():returnText(false)
        mainWindow:getCliCommand():commandActive(false)
        self.finished = true
    end
end

function CreateOperations:manualClose()
    self:close() --if not defined manually
end
