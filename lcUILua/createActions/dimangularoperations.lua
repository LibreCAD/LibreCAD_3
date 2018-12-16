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

    message("Click on center point", id)

    CreateOperations._init(self, id)
end

function DimAngularOperations:getDimAngular(centerPoint, firstPoint, secondPoint, text)
    local layer = active_layer(self.target_widget)
    local viewport = active_viewport(self.target_widget)
    local metaInfo = active_metaInfo(self.target_widget)
    local dim = DimAngular.dimAuto(centerPoint, firstPoint, secondPoint, text, layer, viewport, metaInfo)
    dim:setId(self.dimAngular_id)

    return dim
end

function DimAngularOperations:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(data["position"])
    elseif(eventName == "mouseMove") then
        self:createTempDimAngular(data["position"])
    elseif(eventName == "text") then
        self:setText(data["text"])
    end
end

function DimAngularOperations:newData(data)
    if(self.centerPoint == nil) then
        self.centerPoint = Operations:getCoordinate(data)

        message("Click on first point", self.target_widget)
    elseif(self.firstPoint == nil) then
        self.firstPoint = Operations:getCoordinate(data)

        message("Click on second point", self.target_widget)
    elseif(self.secondPoint == nil) then
        self.secondPoint = Operations:getCoordinate(data)

        message("Enter dimension text or leave it empty (<> for value)", self.target_widget)
        cli_get_text(self.target_widget, true)
    end
end

function DimAngularOperations:setText(text)
    if(text == "") then
        self.text = "<>"
    else
        self.text = text
    end

    cli_get_text(self.target_widget, false)
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