DimLinearOperations = {}
DimLinearOperations.__index = DimLinearOperations

setmetatable(DimLinearOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimLinearOperations:_init(id)
    Operations._init(self, id)

    self.startPoint = nil
    self.endPoint = nil
    self.middleOfText = nil
    self.text = nil

    self.dimLinear_id = ID():id()
    self.dimLinear = self:getDimLinear(Coord(0,0), Coord(1,0), Coord(1,1), "<>")
    self.dimLine = nil

    active_widget():tempEntities():addEntity(self.dimLinear)

    event.register('point', self)
    event.register('mouseMove', self)
    event.register('number', self)
    event.register('text', self)

    message("Click on start point")
end

function DimLinearOperations:getDimLinear(startPoint, endPoint, middleOfText, text)
    local d = active_widget():document()
    local layer = d:layerByName("0")
    local dim = DimLinear.dimAuto(startPoint, endPoint, middleOfText, text, layer, MetaInfo())
    dim:setId(self.dimLinear_id)

    return dim
end

function DimLinearOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:createTempDimLinear(...)
    elseif(eventName == "text") then
        self:setText(...)
    end
end

function DimLinearOperations:newData(data)
    if(self.startPoint == nil) then
        self.startPoint = Operations:getCoordinate(data)

        message("Click on end point")
    elseif(self.endPoint == nil) then
        self.endPoint = Operations:getCoordinate(data)

        message("Give dimension height")
    elseif(self.middleOfText == nil) then
        self.middleOfText = Operations:getCoordinate(data)

        message("Enter dimension text or leave it empty (<> for value)")
        cli_get_text(true)
    end
end

function DimLinearOperations:setText(text)
    if(text == "") then
        self.text = "<>"
    else
        self.text = text
    end

    cli_get_text(false)
    self:createDimLinear()
end

function DimLinearOperations:createTempDimLinear(point)
    local startPoint = self.startPoint
    local endPoint = self.endPoint
    local middleOfText = self.middleOfText

    if(startPoint == nil) then
        startPoint = point
    elseif(endPoint == nil) then
        endPoint = point
    elseif(middleOfText == nil) then
        middleOfText = point
    end

    endPoint = endPoint or startPoint:add(Coord(10,0))
    middleOfText = middleOfText or startPoint:add(Coord(5,10))

    active_widget():tempEntities():removeEntity(self.dimLinear)

    self.dimLinear = self:getDimLinear(startPoint, endPoint, middleOfText, "<>")

    active_widget():tempEntities():addEntity(self.dimLinear)
end

function DimLinearOperations:createDimLinear()
    active_widget():tempEntities():removeEntity(self.dimLinear)

    local b = Builder(active_widget():document())
    local c = self:getDimLinear(self.startPoint, self.endPoint, self.middleOfText, self.text)
    b:append(c)
    b:execute()

    event.delete('mouseMove', self)
    event.delete('number', self)
    event.delete('point', self)
    event.delete('text', self)
end