ScaleOperation = {}
ScaleOperation.__index = ScaleOperation

setmetatable(ScaleOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function ScaleOperation:init()
    mainWindow:getToolbar():addButton("", ":/icons/scale.png", "Modify", 1, 1, function() run_basic_operation(ScaleOperation) end, "Scale")

    mainWindow:getCliCommand():addCommand("SCALE", function() run_basic_operation(ScaleOperation) end)
end

function ScaleOperation:_init(id)
    Operations._init(self)

    self.selection = mainWindow:getCadMdiChild():selection()

    message(tostring(#self.selection) .. " items selected")

    if(#self.selection > 0) then
        self.origin = nil
        self.factor = nil

        self.tempEntities = {}

        luaInterface:registerEvent('point', self)
        luaInterface:registerEvent('number', self)

        message("Give origin point")
    else
        self.finished = true
        luaInterface:triggerEvent('operationFinished')
    end
end

function ScaleOperation:onEvent(eventName, data)
    if(eventName == "point") then
        self:newData(data["position"])
    elseif(eventName == "number") then
        self:newData(data["number"])
    end
end

function ScaleOperation:newData(data)
    if(self.origin == nil) then
        self.origin = Operations:getCoordinate(data)

        message("Enter scale factor or entity end")
    elseif(type(data) == "number") then
        self.factor = lc.geo.Coordinate(data, data, data)

        self:scale()
    end
end

function ScaleOperation:scale()
    local b = lc.operation.EntityBuilder(mainWindow:getCadMdiChild():document())

    for k, entity in pairs(self.selection) do
        b:appendEntity(entity)
    end

    b:appendOperation(lc.operation.Push())
    b:appendOperation(lc.operation.Scale(self.origin, self.factor))
    b:execute()

    self:close()
end

function ScaleOperation:close()
    if(not self.finished) then
        self.finished = true

        for k, entity in pairs(self.tempEntities) do
            mainWindow:getCadMdiChild():tempEntities():removeEntity(entity)
        end

        luaInterface:deleteEvent('point', self)
        luaInterface:deleteEvent('number', self)

        luaInterface:triggerEvent('operationFinished')
    end
end
