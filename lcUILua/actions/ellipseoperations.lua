EllipseOperations = {}
EllipseOperations.__index = EllipseOperations

setmetatable(EllipseOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function EllipseOperations:_init(id, isArc)
    Operations._init(self, id)

    self.isArc = isArc or false
    self.center = nil
    self.majorPoint = nil
    self.minorRadius = nil
    self.startAngle = nil
    self.endAngle = nil

    self.ellipse_id = ID():id()
    self.ellipse = self:getEllipse(Coord(0,0), Coord(0,0), 0, 0, 0)

    active_widget():tempEntities():addEntity(self.ellipse)

    event.register('point', self)
    event.register('mouseMove', self)
    event.register('number', self)

    message("Click on center")
end

function EllipseOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:createTempEllipse(...)
    end
end

function EllipseOperations:newData(data)
    if(self.center == nil) then
        self.center = Operations:getCoordinate(data)

        message("Click on major point")
    elseif(self.majorPoint == nil) then
        self.majorPoint = Operations:getCoordinate(data)

        message("Give minor radius")
    elseif(self.minorRadius == nil) then
        self.minorRadius = Operations:getDistance(self.center, data)

        if(not self.isArc) then
            self:createEllipse()
        else
            message("Enter start angle")
        end
    elseif(self.startAngle == nil) then
        self.startAngle = Operations:getAngle(self.center, data)
        message("Enter end angle")
    elseif(self.endAngle == nil) then
        self.endAngle = Operations:getAngle(self.center, data)
        self:createEllipse()
    end
end

function EllipseOperations:getEllipse(center, majorPoint, minorRadius, startAngle, endAngle)
    local layer = active_layer()
    local metaInfo = active_metaInfo()
    local e = Ellipse(center, majorPoint, minorRadius, startAngle, endAngle, false, layer, metaInfo)
    e:setId(self.ellipse_id)

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
        majorPoint = point
        minorRadius = Operations:getDistance(center, majorPoint) / 2
    elseif(minorRadius == nil) then
        minorRadius = Operations:getDistance(center, point)
    elseif(startAngle == nil) then
        startAngle = Operations:getAngle(self.center, point)
    elseif(endAngle == nil) then
        endAngle = Operations:getAngle(self.center, point)
    end

    majorPoint = majorPoint or Coord(0,0)
    minorRadius = minorRadius or 0
    startAngle = startAngle or 0.001
    endAngle = endAngle or 0

    active_widget():tempEntities():removeEntity(self.ellipse)

    self.ellipse = self:getEllipse(center, majorPoint, minorRadius, startAngle, endAngle)

    active_widget():tempEntities():addEntity(self.ellipse)
end

function EllipseOperations:createEllipse()
    self.finished = true
    active_widget():tempEntities():removeEntity(self.ellipse)

    if(not self.isArc) then
        self.startAngle = 0.001
        self.endAngle = 0
    end

    local b = Builder(active_widget():document())
    local c = self:getEllipse(self.center, self.majorPoint, self.minorRadius, self.startAngle, self.endAngle)
    b:append(c)
    b:execute()

    event.delete('mouseMove', self)
    event.delete('number', self)
    event.delete('point', self)
end

function EllipseOperations:close()
    if(not self.finished) then
        active_widget():tempEntities():removeEntity(self.ellipse)
        self.finished = true

        event.delete('mouseMove', self)
        event.delete('number', self)
        event.delete('point', self)
    end
end