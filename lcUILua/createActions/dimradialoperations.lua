DimRadialOperations = {}
DimRadialOperations.__index = DimRadialOperations

setmetatable(DimRadialOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimRadialOperations:_init(id)
    self.definitionPoint = nil
    self.definitionPoint2 = nil
    self.middleOfText = nil
    self.text = nil

    self.dimRadial_id = ID():id()

    message("Click on first definition point", id)

    CreateOperations._init(self, id)
end

function DimRadialOperations:getDimRadial(defPoint1, defPoint2, middleOfText, text)
    local layer = active_layer(self.target_widget)
    local metaInfo = active_metaInfo(self.target_widget)
    local dim = DimRadial(defPoint1, middleOfText, 5, 0, 1, 1, text, defPoint2, 1, layer, metaInfo)
    dim:setId(self.dimRadial_id)

    return dim
end

function DimRadialOperations:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(data["position"])
    elseif(eventName == "mouseMove") then
        self:createTempDimRadial(data["position"])
    elseif(eventName == "text") then
        self:setText(data["text"])
    end
end

function DimRadialOperations:newData(data)
    if(self.definitionPoint == nil) then
        self.definitionPoint = Operations:getCoordinate(data)

        message("Click on second definition point", self.target_widget)
    elseif(self.definitionPoint2 == nil) then
        self.definitionPoint2 = Operations:getCoordinate(data)

        message("Click on text emplacement", self.target_widget)
    elseif(self.middleOfText == nil) then
        self.middleOfText = Operations:getCoordinate(data)

        message("Enter dimension text or leave it empty (<> for value)", self.target_widget)
        cli_get_text(self.target_widget, true)
    end
end

function DimRadialOperations:setText(text)
    if(text == "") then
        self.text = "<>"
    else
        self.text = text
    end

    cli_get_text(self.target_widget, false)
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

    self.entity = self:getDimRadial(definitionPoint, definitionPoint2, middleOfText, "<>")
    CreateOperations.refreshTempEntity(self)
end

function DimRadialOperations:createDimRadial()
    CreateOperations.createEntity(self, self:getDimRadial(self.definitionPoint, self.definitionPoint2, self.middleOfText, self.text))
    CreateOperations.close(self)
end

function DimRadialOperations:close()
    if(not self.finished) then
        if(self.definitionPoint ~= nil and self.definitionPoint2 ~= nil and self.middleOfText ~= nil) then
            self.text = "<>"
            self:createDimRadial()
        end
    end

    CreateOperations.close(self)
end