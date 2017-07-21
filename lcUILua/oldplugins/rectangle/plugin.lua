local Rectangle = {}
Rectangle.__index = Rectangle

setmetatable(Rectangle, {
    __index = Operations,
    __call = function (cls, ...)
        return cls.new(...)
    end,
})

function Rectangle.new(id)
    local self = setmetatable({}, Rectangle)

    Operations._init(self, id)

    self.x1 = nil
    self.x2 = nil
    self.y1 = nil
    self.y2 = nil

    self.entities = {}

    event.register("point", self)
    event.register("mouseMove", self)

    message("Enter corner 1")

    return self
end

function Rectangle:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:tempRectangle(...)
    end
end

function Rectangle:newData(data)
    if(self.x1 == nil or self.y1 == nil) then
        self.x1 = data:x()
        self.y1 = data:y()
        message("Enter corner 2")
    elseif(self.x2 == nil or self.y2 == nil) then
        self.x2 = data:x()
        self.y2 = data:y()
        self:drawRectangle()
    end
end

function Rectangle:calc(x1, y1, x2, y2)
    local entities = {}
    table.insert(entities, Line(Coord(x1,y1), Coord(x2,y1), active_layer(), active_metaInfo()))
    table.insert(entities, Line(Coord(x2,y1), Coord(x2,y2), active_layer(), active_metaInfo()))
    table.insert(entities, Line(Coord(x2,y2), Coord(x1,y2), active_layer(), active_metaInfo()))
    table.insert(entities, Line(Coord(x1,y2), Coord(x1,y1), active_layer(), active_metaInfo()))
    return entities
end

function Rectangle:drawRectangle()
    local entities = Rectangle:calc(self.x1, self.y1, self.x2, self.y2)
    local b = Builder(active_widget():document())

    for k, v in pairs(entities) do
        b:append(v)
    end

    b:push()

    b:execute()

    self:close()
end

function Rectangle:tempRectangle(point)
    local x1
    local y1
    local x2
    local y2

    if(self.x1 == nil) then
        x1 = point:x()
        y1 = point:y()
        x2 = point:x() + 10
        y2 = point:y() + 10
    else
        x1 = self.x1
        y1 = self.y1
        x2 = point:x()
        y2 = point:y()
    end

    for k, v in pairs(self.entities) do
        active_widget():tempEntities():removeEntity(v)
    end

    self.entities = self:calc(x1, y1, x2, y2)

    for k, v in pairs(self.entities) do
        active_widget():tempEntities():addEntity(v)
    end
end

function Rectangle:close()
    if(self.finished == false) then
        self.finished = true

        for k, v in pairs(self.entities) do
            active_widget():tempEntities():removeEntity(v)
        end

        event.delete("point", self)
        event.delete("mouseMove", self)

        event.trigger("operationFinished")
    end
end