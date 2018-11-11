DimAlignedOperations = {}
DimAlignedOperations.__index = DimAlignedOperations

setmetatable(DimAlignedOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function DimAlignedOperations:_init(id)
    self.startPoint = nil
    self.endPoint = nil
    self.middleOfText = nil
    self.text = nil

    self.dimLine = nil

    message("Click on start point", id)

    CreateOperations._init(self, id)
end

function DimAlignedOperations:getDimAligned(startPoint, endPoint, middleOfText, text)
    local layer = active_layer(self.target_widget)
    local metaInfo = active_metaInfo(self.target_widget)
    local dim = lc.entity.DimAligned.dimAuto(startPoint, endPoint, middleOfText, text, layer, metaInfo)

    return dim
end

function DimAlignedOperations:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(data["position"])
    elseif(eventName == "mouseMove") then
        self:createTempDimAligned(data["position"])
    elseif(eventName == "text") then
        self:setText(data["text"])
    end
end

function DimAlignedOperations:newData(data)
    if(self.startPoint == nil) then
        self.startPoint = Operations:getCoordinate(data)

        message("Click on end point", self.target_widget)
    elseif(self.endPoint == nil) then
        local metaInfo = active_metaInfo(self.target_widget)
        local layer = active_layer(self.target_widget)

        self.endPoint = Operations:getCoordinate(data)
        self.dimLine = lc.entity.Line(self.startPoint, self.endPoint, layer, metaInfo)

        message("Give dimension height", self.target_widget)
    elseif(self.middleOfText == nil) then
        self.middleOfText = Operations:getCoordinate(data)

        message("Enter dimension text or leave it empty (<> for value)", self.target_widget)
        cli_get_text(self.target_widget, true)
    end
end

function DimAlignedOperations:setText(text)
    if(text == "") then
        self.text = "<>"
    else
        self.text = text
    end

    cli_get_text(self.target_widget, false)
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
            endPoint = endPoint:add(lc.geo.Coordinate(0.001,0.001))
        end
    elseif(middleOfText == nil) then
        middleOfText = point
    end

    endPoint = endPoint or startPoint:add(lc.geo.Coordinate(10,0))
    middleOfText = middleOfText or startPoint:add(lc.geo.Coordinate(5,10))

    self.entity = self:getDimAligned(startPoint, endPoint, middleOfText, "<>")

    CreateOperations.refreshTempEntity(self)
end

function DimAlignedOperations:createDimAligned()
    CreateOperations.createEntity(self, self:getDimAligned(self.startPoint, self.endPoint, self.middleOfText, self.text))
    CreateOperations.close(self)
end

function DimAlignedOperations:close()
    if(not self.finished) then
        if(self.startPoint ~= nil and self.endPoint ~= nil and self.middleOfText ~= nil) then
            self.text = "<>"
            self:createDimAligned()
        end
    end

    CreateOperations.close(self)
end