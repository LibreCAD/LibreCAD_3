DimDiametricOperations = {}
DimDiametricOperations.__index = DimDiametricOperations

setmetatable(DimDiametricOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimDiametricOperations:_init(id)
    self.definitionPoint = nil
    self.definitionPoint2 = nil
    self.text = nil

    self.dimDiametric_id = ID():id()

    message("Click on first point")

    CreateOperations._init(self, id)
end

function DimDiametricOperations:getDimDiametric(defPoint1, defPoint2, text)
    local layer = active_layer()
    local metaInfo = active_metaInfo()
    local dim = DimDiametric(defPoint1, 5, 1, 1, text, defPoint2, 1, layer, metaInfo)
    dim:setId(self.dimDiametric_id)

    return dim
end

function DimDiametricOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:createTempDimDiametric(...)
    elseif(eventName == "text") then
        self:setText(...)
    end
end

function DimDiametricOperations:newData(data)
    if(self.definitionPoint == nil) then
        self.definitionPoint = Operations:getCoordinate(data)

        message("Click on second point")
    elseif(self.definitionPoint2 == nil) then
        self.definitionPoint2 = Operations:getCoordinate(data)

        message("Enter dimension text or leave it empty (<> for value)")
        cli_get_text(true)
    end
end

function DimDiametricOperations:setText(text)
    if(text == "") then
        self.text = "<>"
    else
        self.text = text
    end

    cli_get_text(false)
    self:createDimDiametric()
end

function DimDiametricOperations:createTempDimDiametric(point)
    local definitionPoint = self.definitionPoint
    local definitionPoint2 = self.definitionPoint2

    if(definitionPoint == nil) then
        definitionPoint = point
    elseif(definitionPoint2 == nil) then
        definitionPoint2 = point

        if(definitionPoint:x() == definitionPoint2:x() and definitionPoint:y() == definitionPoint2:y()) then
            definitionPoint2 = definitionPoint2:add(Coord(0.001,0.001))
        end
    end

    definitionPoint2 = definitionPoint2 or definitionPoint:add(Coord(10,0))

    self.entity = self:getDimDiametric(definitionPoint, definitionPoint2, "<>")
    CreateOperations.refreshTempEntity(self)
end

function DimDiametricOperations:createDimDiametric()
    CreateOperations:createEntity(self:getDimDiametric(self.definitionPoint, self.definitionPoint2, self.text))
    CreateOperations.close(self)
end

function DimDiametricOperations:close()
    if(not self.finished) then
        if(self.definitionPoint ~= nil and self.definitionPoint2 ~= nil) then
            self.text = "<>"
            self:createDimDiametric()
        end
    end

    CreateOperations.close(self)
end