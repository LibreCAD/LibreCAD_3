DimAlignedOperations = {}
DimAlignedOperations.__index = DimAlignedOperations

setmetatable(DimAlignedOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimAlignedOperations:_init(id)
    Operations._init(self, id)

    self.startPoint = nil
    self.endPoint = nil
    self.middleOfText = nil
    self.text = nil

    self.dimAligned_id = ID():id()
    self.dimAligned = self:getDimAligned(Coord(0,0), Coord(1,0), Coord(1,1), "<>")
    self.dimLine = nil

    active_widget():tempEntities():addEntity(self.dimAligned)

    event.register('point', self)
    event.register('mouseMove', self)
    event.register('number', self)
    event.register('text', self)

    message("Click on start point")
end

function DimAlignedOperations:getDimAligned(startPoint, endPoint, middleOfText, text)
    local d = active_widget():document()
    local layer = d:layerByName("0")
    local dim = DimAligned.dimAuto(startPoint, endPoint, middleOfText, text, layer, MetaInfo())
    dim:setId(self.dimAligned_id)

    return dim
end

function DimAlignedOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:createTempDimAligned(...)
    elseif(eventName == "text") then
        self:setText(...)
    end
end

function DimAlignedOperations:newData(data)
    if(self.startPoint == nil) then
        self.startPoint = Operations:getCoordinate(data)

        message("Click on end point")
    elseif(self.endPoint == nil) then
        self.endPoint = Operations:getCoordinate(data)
        self.dimLine = Line(self.startPoint, self.endPoint, Layer("Temp", Color(0,0,0,0)))

        message("Give dimension height")
    elseif(self.middleOfText == nil) then
        self.middleOfText = Operations:getCoordinate(data)

        message("Enter dimension text or leave it empty (<> for value)")
        cli_get_text(true)
    end
end

function DimAlignedOperations:setText(text)
    if(text == "") then
        self.text = "<>"
    else
        self.text = text
    end

    cli_get_text(false)
    self:createDimAligned()
end

function DimAlignedOperations:createTempDimAligned(point)
    local startPoint = self.startPoint
    local endPoint = self.endPoint
    local middleOfText = self.middleOfText

    if(startPoint == nil) then
        startPoint = point
    elseif(endPoint == nil) then
        endPoint = point

        if(startPoint:x() == endPoint:x() and startPoint:y() == endPoint:y()) then
            endPoint = endPoint:add(Coord(0.001,0.001))
        end
    elseif(middleOfText == nil) then
        middleOfText = point
    end

    endPoint = endPoint or startPoint:add(Coord(10,0))
    middleOfText = middleOfText or startPoint:add(Coord(5,10))

    active_widget():tempEntities():removeEntity(self.dimAligned)

    self.dimAligned = self:getDimAligned(startPoint, endPoint, middleOfText, "<>")

    active_widget():tempEntities():addEntity(self.dimAligned)
end

function DimAlignedOperations:createDimAligned()
    active_widget():tempEntities():removeEntity(self.dimAligned)

    local b = Builder(active_widget():document())
    local c = self:getDimAligned(self.startPoint, self.endPoint, self.middleOfText, self.text)
    b:append(c)
    b:execute()

    event.delete('mouseMove', self)
    event.delete('number', self)
    event.delete('point', self)
    event.delete('text', self)
end