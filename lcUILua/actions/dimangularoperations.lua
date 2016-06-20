DimAngularOperations = {}
DimAngularOperations.__index = DimAngularOperations

setmetatable(DimAngularOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimAngularOperations:_init(id)
    Operations._init(self, id)

    self.startPoint = nil
    self.endPoint = nil
    self.textOffset = nil
    self.text = nil

    self.dimAngular_id = ID():id()
    self.dimAngular = self:getDimAngular(Coord(0,0), Coord(1,1), 1, "<>")
    self.dimLine = nil

    active_widget():tempEntities():addEntity(self.dimAngular)

    event.register('point', self)
    event.register('mouseMove', self)
    event.register('number', self)
    event.register('text', self)

    message("Todo: fix DimAuto for DimAngular")
    message("Click on start point")
end

function DimAngularOperations:getDimAngular(startPoint, endPoint, textOffset, text)
    local d = active_widget():document()
    local layer = d:layerByName("0")
    local dim = DimAngular.dimAuto(startPoint, endPoint, textOffset, text, layer, MetaInfo())
    dim:setId(self.dimAngular_id)

    return dim
end

function DimAngularOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:createTempDimAngular(...)
    elseif(eventName == "text") then
        self:setText(...)
    end
end

function DimAngularOperations:newData(data)
    if(self.startPoint == nil) then
        self.startPoint = Operations:getCoordinate(data)

        message("Click on end point")
    elseif(self.endPoint == nil) then
        self.endPoint = Operations:getCoordinate(data)
        self.dimLine = Line(self.startPoint, self.endPoint, Layer("Temp", Color(0,0,0,0)))

        message("Give dimension height")
    elseif(self.textOffset == nil) then
        if(type(data) == "userdata") then
            self.textOffset = self.dimLine:nearestPointOnPath(data):distanceTo(data)
        else
            self.textOffset = data
        end

        message("Enter dimension text or leave it empty (<> for value)")
        cli_get_text(true)
    end
end

function DimAngularOperations:setText(text)
    if(text == "") then
        self.text = "<>"
    else
        self.text = text
    end

    cli_get_text(false)
    self:createDimAngular()
end

function DimAngularOperations:createTempDimAngular(point)
    local startPoint = self.startPoint
    local endPoint = self.endPoint
    local textOffset = self.textOffset

    if(startPoint == nil) then
        startPoint = point
    elseif(endPoint == nil) then
        endPoint = point

        if(startPoint:x() == endPoint:x() and startPoint:y() == endPoint:y()) then
            endPoint = endPoint:add(Coord(0.001,0.001))
        end
    elseif(textOffset == nil) then
        textOffset = self.dimLine:nearestPointOnPath(point):distanceTo(point)
    end

    endPoint = endPoint or startPoint:add(Coord(10,0))
    textOffset = textOffset or 10

    active_widget():tempEntities():removeEntity(self.dimAngular)

    self.dimAngular = self:getDimAngular(startPoint, endPoint, textOffset, "<>")

    active_widget():tempEntities():addEntity(self.dimAngular)
end

function DimAngularOperations:createDimAngular()
    active_widget():tempEntities():removeEntity(self.dimAngular)

    local b = Builder(active_widget():document())
    local c = self:getDimAngular(self.startPoint, self.endPoint, self.textOffset, self.text)
    b:append(c)
    b:execute()

    event.delete('mouseMove', self)
    event.delete('number', self)
    event.delete('point', self)
    event.delete('text', self)
end