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

function ScaleOperation:_init(id)
    Operations._init(self, id)

    self.selection = active_widget():selection()

    message(tostring(#self.selection) .. " items selected")

    if(#self.selection > 0) then
        self.origin = nil
        self.factor = nil

        self.tempEntities = {}

        event.register('point', self)
        event.register('number', self)

        message("Give origin point")
    else
        self.finished = true
        event.trigger('operationFinished')
    end
end

function ScaleOperation:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    end
end

function ScaleOperation:newData(data)
    if(self.origin == nil) then
        self.origin = Operations:getCoordinate(data)

        message("Enter scale factor or entity end")
    elseif(type(data) == "number") then
        self.factor = Coordinate(data, data, data)

        self:scale()
    end
end

function ScaleOperation:scale()
    local b = EntityBuilder(active_widget():document())

    for k, entity in pairs(self.selection) do
        b:appendEntity(entity)
    end

    b:appendOperation(Push())
    b:appendOperation(Scale(self.origin, self.factor))
    b:execute()

    self:close()
end

function ScaleOperation:close()
    if(not self.finished) then
        self.finished = true

        for k, entity in pairs(self.tempEntities) do
            active_widget():tempEntities():removeEntity(entity)
        end

        event.delete('point', self)
        event.delete('number', self)

        event.trigger('operationFinished')
    end
end