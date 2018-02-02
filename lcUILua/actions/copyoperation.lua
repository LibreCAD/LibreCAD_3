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

function CopyOperation:_init(widget)
    Operations._init(self, widget)

    self.selection = getWindow(widget):selection()

    message(tostring(#self.selection) .. " items selected", self.target_widget)

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
    if(Operations.forMe(self, event) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(event["position"])
    elseif(eventName == "mouseMove") then
        self:tempCopy(event["position"])
    end
end

function CopyOperation:newData(point)
    if(self.origin == nil) then
        self.origin = Operations:getCoordinate(point)

        message("Give destination point", self.target_widget)
    elseif(Operations:getCoordinate(point) ~= nil) then
        self.destination = Operations:getCoordinate(point)

        self:copy()
    end
end

function CopyOperation:tempCopy(point)
    if(self.origin ~= nil) then
        local window = getWindow(self.target_widget)
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
    local b = EntityBuilder(getWindow(self.target_widget):document())

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
        local window = getWindow(self.target_widget)
        for k, entity in pairs(self.tempEntities) do
            window:tempEntities():removeEntity(entity)
        end

        luaInterface:deleteEvent('mouseMove', self)
        luaInterface:deleteEvent('point', self)

        luaInterface:triggerEvent('operationFinished', self.target_widget)
    end
end