RotateOperation = {
    command_line = "ROTATE"
}
RotateOperation.__index = RotateOperation

setmetatable(RotateOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function RotateOperation:init()
    mainWindow:getToolbar():addButton("", ":/icons/modifyrotate.png", "Modify", 1, 0, function() run_basic_operation(RotateOperation) end, "Rotate")
end

function RotateOperation:_init(id)
    Operations._init(self)

    self.selection = mainWindow:getCadMdiChild():selection()

    message(tostring(#self.selection) .. " items selected")

    if(#self.selection > 0) then
        self.origin = nil
        self.firstPoint = nil
        self.secondPoint = nil

        self.tempEntities = {}

        luaInterface:registerEvent('point', self)
        luaInterface:registerEvent('mouseMove', self)

        message("Give origin point")
    else
        self.finished = true
        luaInterface:triggerEvent('operationFinished')
    end
end

function RotateOperation:onEvent(eventName, data)
    if(eventName == "point" or eventName == "number") then
        self:newData(data["position"])
    elseif(eventName == "mouseMove") then
        self:tempRotate(data["position"])
    end
end

function RotateOperation:newData(point)
    if(self.origin == nil) then
        self.origin = Operations:getCoordinate(point)

        message("Give first point")
    elseif(self.firstPoint == nil) then
        self.firstPoint = Operations:getCoordinate(point)

        message("Give second point")
    elseif(Operations:getCoordinate(point) ~= nil) then
        self.secondPoint = Operations:getCoordinate(point)

        self:rotate()
    end
end

function RotateOperation:tempRotate(point)
    if(self.origin ~= nil and self.firstPoint ~= nil) then
        local window = mainWindow:getCadMdiChild()

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
    local b = lc.operation.EntityBuilder(mainWindow:getCadMdiChild():document())

    for k, entity in pairs(self.selection) do
        b:appendEntity(entity)
    end

    b:appendOperation(lc.operation.Push())
    b:appendOperation(lc.operation.Rotate(self.origin, angle))
    b:execute()

    self:close()
end

function RotateOperation:close()
    if(not self.finished) then
        self.finished = true

        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end

        luaInterface:deleteEvent('mouseMove', self)
        luaInterface:deleteEvent('point', self)

        luaInterface:triggerEvent('operationFinished')
    end
end
