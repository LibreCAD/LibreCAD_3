DimLinearOperations = {}
DimLinearOperations.__index = DimLinearOperations

setmetatable(DimLinearOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimLinearOperations:_init(id)
    self.startPoint = nil
    self.endPoint = nil
    self.middleOfText = nil
    self.text = nil

    self.dimLinear_id = ID():id()
    self.dimLine = nil

    message("Click on start point", id)

    CreateOperations._init(self, id)
end

function DimLinearOperations:getDimLinear(startPoint, endPoint, middleOfText, text)
    local layer = active_layer(self.target_widget)
    local metaInfo = active_metaInfo(self.target_widget)
    local dim = DimLinear.dimAuto(startPoint, endPoint, middleOfText, text, layer, metaInfo)
    dim:setId(self.dimLinear_id)

    return dim
end

function DimLinearOperations:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(data["position"])
    elseif(eventName == "mouseMove") then
        self:createTempDimLinear(data["position"])
    elseif(eventName == "text") then
        self:setText(data["text"])
    end
end

function DimLinearOperations:newData(data)
    if(self.startPoint == nil) then
        self.startPoint = Operations:getCoordinate(data)

        message("Click on end point", self.target_widget)
    elseif(self.endPoint == nil) then
        self.endPoint = Operations:getCoordinate(data)

        message("Give dimension height", self.target_widget)
    elseif(self.middleOfText == nil) then
        self.middleOfText = Operations:getCoordinate(data)

        message("Enter dimension text or leave it empty (<> for value)", self.target_widget)
        cli_get_text(self.target_widget, true)
    end
end

function DimLinearOperations:setText(text)
    if(text == "") then
        self.text = "<>"
    else
        self.text = text
    end

    cli_get_text(self.target_widget, false)
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

    self.entity = self:getDimLinear(startPoint, endPoint, middleOfText, "<>")
    CreateOperations.refreshTempEntity(self)
end

function DimLinearOperations:createDimLinear()
    CreateOperations.createEntity(self, self:getDimLinear(self.startPoint, self.endPoint, self.middleOfText, self.text))
    CreateOperations.close(self)
end

function DimLinearOperations:close()
    if(not self.finished) then
        if(self.startPoint ~= nil and self.endPoint ~= nil and self.middleOfText ~= nil) then
            self.text = "<>"
            self:createDimLinear()
        end
    end

    CreateOperations.close(self)
end