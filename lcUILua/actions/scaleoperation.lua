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

    self.selection = getWindow(id):selection()

    message(tostring(#self.selection) .. " items selected", id)

    if(#self.selection > 0) then
        self.origin = nil
        self.factor = nil

        self.tempEntities = {}

        luaInterface:registerEvent('point', self)
        luaInterface:registerEvent('number', self)

        message("Give origin point")
    else
        self.finished = true
        luaInterface:triggerEvent('operationFinished', id)
    end
end

function ScaleOperation:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(eventName == "point") then
        self:newData(data["position"])
    elseif(eventName == "number") then
        self:newData(data["number"])
    end
end

function ScaleOperation:newData(data)
    if(self.origin == nil) then
        self.origin = Operations:getCoordinate(data)

        message("Enter scale factor or entity end", self.target_widget)
    elseif(type(data) == "number") then
        self.factor = Coordinate(data, data, data)

        self:scale()
    end
end

function ScaleOperation:scale()
    local b = EntityBuilder(getWindow(self.target_widget):document())

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
            getWindow(self.target_widget):tempEntities():removeEntity(entity)
        end

        luaInterface:deleteEvent('point', self)
        luaInterface:deleteEvent('number', self)

        luaInterface:triggerEvent('operationFinished', self.target_widget)
    end
end