DimAngularOperations = {}
DimAngularOperations.__index = DimAngularOperations

setmetatable(DimAngularOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimAngularOperations:_init(id)
    self.centerPoint = nil
    self.firstPoint = nil
    self.secondPoint = nil
    self.text = nil

    self.dimAngular_id = ID():id()
    self.dimLine = nil

    CreateOperations.refreshTempEntity(self)

    message("Click on center point")

    CreateOperations._init(self, id)
end

function DimAngularOperations:getDimAngular(centerPoint, firstPoint, secondPoint, text)
    local layer = active_layer()
    local metaInfo = active_metaInfo()
    local dim = DimAngular.dimAuto(centerPoint, firstPoint, secondPoint, text, layer, metaInfo)
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
    if(self.centerPoint == nil) then
        self.centerPoint = Operations:getCoordinate(data)

        message("Click on first point")
    elseif(self.firstPoint == nil) then
        self.firstPoint = Operations:getCoordinate(data)

        message("Click on second point")
    elseif(self.secondPoint == nil) then
        self.secondPoint = Operations:getCoordinate(data)

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
    local centerPoint = self.centerPoint
    local firstPoint = self.firstPoint
    local secondPoint = self.secondPoint

    if(centerPoint == nil) then
        centerPoint = point
    elseif(firstPoint == nil) then
        firstPoint = point
    elseif(secondPoint == nil) then
        secondPoint = point
    end

    firstPoint = firstPoint or centerPoint:add(Coord(-1, 1))
    secondPoint = secondPoint or centerPoint:add(Coord(1, 1))

    self.entity = self:getDimAngular(centerPoint, firstPoint, secondPoint, "<>")
    CreateOperations.refreshTempEntity(self)
end

function DimAngularOperations:createDimAngular()
    CreateOperations.createEntity(self, self:getDimAngular(self.centerPoint, self.firstPoint, self.secondPoint, self.text))
    CreateOperations.close(self)
end

function DimAngularOperations:close()
    if(not self.finished) then
        if(self.centerPoint ~= nil and self.firstPoint ~= nil and self.secondPoint ~= nil) then
            self.text = "<>"
            self:createDimAngular()
        end
    end

    CreateOperations.close(self)
end