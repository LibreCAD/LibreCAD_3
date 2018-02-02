RotateOperation = {}
RotateOperation.__index = RotateOperation

setmetatable(RotateOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function RotateOperation:_init(id)
    Operations._init(self, id)

    self.selection = getWindow(id):selection()

    message(tostring(#self.selection) .. " items selected", id)

    if(#self.selection > 0) then
        self.origin = nil
        self.firstPoint = nil
        self.secondPoint = nil

        self.tempEntities = {}

        luaInterface:registerEvent('point', self)
        luaInterface:registerEvent('mouseMove', self)

        message("Give origin point", id)
    else
        self.finished = true
        luaInterface:triggerEvent('operationFinished', id)
    end
end

function RotateOperation:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(data["position"])
    elseif(eventName == "mouseMove") then
        self:tempRotate(data["position"])
    end
end

function RotateOperation:newData(point)
    if(self.origin == nil) then
        self.origin = Operations:getCoordinate(point)

        message("Give first point", self.target_widget)
    elseif(self.firstPoint == nil) then
        self.firstPoint = Operations:getCoordinate(point)

        message("Give second point", self.target_widget)
    elseif(Operations:getCoordinate(point) ~= nil) then
        self.secondPoint = Operations:getCoordinate(point)

        self:rotate()
    end
end

function RotateOperation:tempRotate(point)
    if(self.origin ~= nil and self.firstPoint ~= nil) then
        local window = getWindow(self.target_widget)

        for k, entity in pairs(self.tempEntities) do
            window:tempEntities():removeEntity(entity)
        end
        self.tempEntities = {}

        local angle = self.origin:angleTo(point) - self.origin:angleTo(self.firstPoint)

        for k, entity in pairs(self.selection) do
            local newEntity = entity:rotate(self.origin, angle)
            window:tempEntities():addEntity(newEntity)
            table.insert(self.tempEntities, newEntity)
        end
    end
end

function RotateOperation:rotate()
    local angle = self.origin:angleTo(self.secondPoint) - self.origin:angleTo(self.firstPoint)
    local b = EntityBuilder(getWindow(self.target_widget):document())

    for k, entity in pairs(self.selection) do
        b:appendEntity(entity)
    end

    b:appendOperation(Push())
    b:appendOperation(Rotate(self.origin, angle))
    b:execute()

    self:close()
end

function RotateOperation:close()
    if(not self.finished) then
        self.finished = true

        for k, entity in pairs(self.tempEntities) do
            getWindow(self.target_widget):tempEntities():removeEntity(entity)
        end

        luaInterface:deleteEvent('mouseMove', self)
        luaInterface:deleteEvent('point', self)

        luaInterface:triggerEvent('operationFinished', self.target_widget)
    end
end