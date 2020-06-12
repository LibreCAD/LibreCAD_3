CopyOperation = {
    command_line = "COPY",
    icon = "move_copy.svg"
}
CopyOperation.__index = CopyOperation

setmetatable(CopyOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function CopyOperation:_init(widget)
    Operations._init(self, widget)

    self.selection = mainWindow:cadMdiChild():selection()

    message(tostring(#self.selection) .. " items selected")

    if(#self.selection > 0) then
        self.origin = nil
        self.destination = nil

        self.tempEntities = {}

        luaInterface:registerEvent('point', self)
        luaInterface:registerEvent('mouseMove', self)

        message("Give origin point", widget)
    else
        self.finished = true
        luaInterface:triggerEvent('operationFinished', widget)
    end
end

function CopyOperation:onEvent(eventName, event)
    if(eventName == "point" or eventName == "number") then
        self:newData(event["position"])
    elseif(eventName == "mouseMove") then
        self:tempCopy(event["position"])
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
        local window = mainWindow:cadMdiChild()
        for k, entity in pairs(self.tempEntities) do
            window:tempEntities():removeEntity(entity)
        end
        self.tempEntities = {}

        local offset = point:sub(self.origin)

        for k, entity in pairs(self.selection) do
            local newEntity = entity:copy(offset)
            window:tempEntities():addEntity(newEntity)
            table.insert(self.tempEntities, newEntity)
        end
    end
end

function CopyOperation:copy()
    local offset = self.destination:sub(self.origin)
    local b = lc.operation.EntityBuilder(mainWindow:cadMdiChild():document())

    for k, entity in pairs(self.selection) do
        b:appendEntity(entity)
    end

    b:appendOperation(lc.operation.Push())
    b:appendOperation(lc.operation.Copy(offset))
    b:execute()

    self:close()
end

function CopyOperation:close()
    if(not self.finished) then
        self.finished = true
        local window = mainWindow:cadMdiChild()
        for k, entity in pairs(self.tempEntities) do
            window:tempEntities():removeEntity(entity)
        end

        luaInterface:deleteEvent('mouseMove', self)
        luaInterface:deleteEvent('point', self)

        luaInterface:triggerEvent('operationFinished')
    end
end
