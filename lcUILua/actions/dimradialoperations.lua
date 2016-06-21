DimRadialOperations = {}
DimRadialOperations.__index = DimRadialOperations

setmetatable(DimRadialOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimRadialOperations:_init(id)
    Operations._init(self, id)

    self.definitionPoint = nil
    self.definitionPoint2 = nil
    self.middleOfText = nil
    self.text = nil

    self.dimRadial_id = ID():id()
    self.dimRadial = self:getDimRadial(Coord(0,0), Coord(0,1), Coord(1,1), "<>")

    active_widget():tempEntities():addEntity(self.dimRadial)

    event.register('point', self)
    event.register('mouseMove', self)
    event.register('number', self)
    event.register('text', self)

    message("Click on first definition point")
end

function DimRadialOperations:getDimRadial(defPoint1, defPoint2, middleOfText, text)
    local d = active_widget():document()
    local layer = d:layerByName("0")
    local dim = DimRadial(defPoint1, middleOfText, 5, 0, 1, 1, text, defPoint2, 1, layer, MetaInfo())
    dim:setId(self.dimRadial_id)

    return dim
end

function DimRadialOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:createTempDimRadial(...)
    elseif(eventName == "text") then
        self:setText(...)
    end
end

function DimRadialOperations:newData(data)
    if(self.definitionPoint == nil) then
        self.definitionPoint = Operations:getCoordinate(data)

        message("Click on second definition point")
    elseif(self.definitionPoint2 == nil) then
        self.definitionPoint2 = Operations:getCoordinate(data)

        message("Click on text emplacement")
    elseif(self.middleOfText == nil) then
        self.middleOfText = Operations:getCoordinate(data)

        message("Enter dimension text or leave it empty (<> for value)")
        cli_get_text(true)
    end
end

function DimRadialOperations:setText(text)
    if(text == "") then
        self.text = "<>"
    else
        self.text = text
    end

    cli_get_text(false)
    self:createDimRadial()
end

function DimRadialOperations:createTempDimRadial(point)
    local definitionPoint = self.definitionPoint
    local definitionPoint2 = self.definitionPoint2
    local middleOfText = self.middleOfText

    if(definitionPoint == nil) then
        definitionPoint = point
    elseif(definitionPoint2 == nil) then
        definitionPoint2 = point

        if(definitionPoint:x() == definitionPoint2:x() and definitionPoint:y() == definitionPoint2:y()) then
            definitionPoint2 = definitionPoint2:add(Coord(0.001,0.001))
        end
    elseif(middleOfText == nil) then
        middleOfText = point
    end

    definitionPoint2 = definitionPoint2 or definitionPoint:add(Coord(10,0))
    middleOfText = middleOfText or definitionPoint:add(Coord(5,10))

    active_widget():tempEntities():removeEntity(self.dimRadial)

    self.dimRadial = self:getDimRadial(definitionPoint, definitionPoint2, middleOfText, "<>")

    active_widget():tempEntities():addEntity(self.dimRadial)
end

function DimRadialOperations:createDimRadial()
    self.finished = true
    active_widget():tempEntities():removeEntity(self.dimRadial)

    local b = Builder(active_widget():document())
    local c = self:getDimRadial(self.definitionPoint, self.definitionPoint2, self.middleOfText, self.text)
    b:append(c)
    b:execute()

    event.delete('mouseMove', self)
    event.delete('number', self)
    event.delete('point', self)
    event.delete('text', self)
end

function DimRadialOperations:close()
    if(not self.finished) then
        if(self.definitionPoint ~= nil and self.definitionPoint2 ~= nil and self.middleOfText ~= nil) then
            self.text = "<>"
            self:createDimRadial()
        else
            active_widget():tempEntities():removeEntity(self.dimRadial)
            self.finished = true

            event.delete('mouseMove', self)
            event.delete('number', self)
            event.delete('point', self)
            event.delete('text', self)
        end
    end
end