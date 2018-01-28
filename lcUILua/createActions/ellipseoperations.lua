EllipseOperations = {}
EllipseOperations.__index = EllipseOperations

setmetatable(EllipseOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function EllipseOperations:_init(id, isArc)
    self.isArc = isArc or false
    self.center = nil
    self.majorPoint = nil
    self.minorRadius = nil
    self.startAngle = nil
    self.endAngle = nil
    self.entity_id = ID():id()

    message("Click on center", id)

    CreateOperations._init(self, id)
end

function EllipseOperations:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end
    if(eventName == "point" or eventName == "number") then
        self:newData(data["position"])
    elseif(eventName == "mouseMove") then
        self:createTempEllipse(data["position"])
    end
end

function EllipseOperations:newData(data)
    if(self.center == nil) then
        self.center = Operations:getCoordinate(data)

        message("Click on major point", self.target_widget)
    elseif(self.majorPoint == nil) then
        self.majorPoint = Operations:getCoordinate(data)
        if(self.majorPoint ~= nil) then
            self.majorPoint = self.majorPoint:sub(self.center)
        end

        message("Give minor radius", self.target_widget)
    elseif(self.minorRadius == nil) then
        self.minorRadius = Operations:getDistance(self.center, data)

        if(not self.isArc) then
            self:createEllipse()
        else
            message("Enter start angle", self.target_widget)
        end
    elseif(self.startAngle == nil) then
        self.startAngle = self.entity:getEllipseAngle(data);
        message("Enter end angle", self.target_widget)
    elseif(self.endAngle == nil) then
        self.endAngle = self.entity:getEllipseAngle(data);
        self:createEllipse()
    end
end

function EllipseOperations:getEllipse(center, majorPoint, minorRadius, startAngle, endAngle)
    local layer = active_layer(self.target_widget)
    local metaInfo = active_metaInfo(self.target_widget)
    local e = Ellipse(center, majorPoint, minorRadius, startAngle, endAngle, false, layer, metaInfo)
    e:setId(self.entity_id)

    return e
end

function EllipseOperations:createTempEllipse(point)
    local center = self.center
    local majorPoint = self.majorPoint
    local minorRadius = self.minorRadius
    local startAngle = self.startAngle
    local endAngle = self.endAngle

    if(center == nil) then
        center = point
    elseif(majorPoint == nil) then
        majorPoint = point:sub(self.center)
        minorRadius = Operations:getDistance(center, majorPoint) / 2
    elseif(minorRadius == nil) then
        minorRadius = Operations:getDistance(center, point)
    elseif(startAngle == nil) then
        startAngle = self.entity:getEllipseAngle(point);
    elseif(endAngle == nil) then
        endAngle = self.entity:getEllipseAngle(point);
    end

    majorPoint = majorPoint or Coord(0,0)
    minorRadius = minorRadius or 0
    startAngle = startAngle or 0.001
    endAngle = endAngle or 0


    self.entity = self:getEllipse(center, majorPoint, minorRadius, startAngle, endAngle)

    self:refreshTempEntity()
end

function EllipseOperations:createEllipse()
    if(not self.isArc) then
        self.startAngle = 0
        self.endAngle = 2 * math.pi
    end

    local el = self:getEllipse(self.center, self.majorPoint, self.minorRadius, self.startAngle, self.endAngle)
    self:createEntity(el)

    CreateOperations.close(self)
end
