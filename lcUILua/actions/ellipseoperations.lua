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

    luaInterface:registerEvent('point', self)
    luaInterface:registerEvent('mouseMove', self)
    luaInterface:registerEvent('number', self)

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
        if(self.majorPoint ~= nil) then
            self.majorPoint = self.majorPoint:sub(self.center)
        end

        message("Give minor radius")
    elseif(self.minorRadius == nil) then
        self.minorRadius = Operations:getDistance(self.center, data)

        if(not self.isArc) then
            self:createEllipse()
        else
            message("Enter start angle")
        end
    elseif(self.startAngle == nil) then
        self.startAngle = self.ellipse:getEllipseAngle(data);
        message("Enter end angle")
    elseif(self.endAngle == nil) then
        self.endAngle = self.ellipse:getEllipseAngle(data);
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
        majorPoint = point:sub(self.center)
        minorRadius = Operations:getDistance(center, majorPoint) / 2
    elseif(minorRadius == nil) then
        minorRadius = Operations:getDistance(center, point)
    elseif(startAngle == nil) then
        startAngle = self.ellipse:getEllipseAngle(point);
    elseif(endAngle == nil) then
        endAngle = self.ellipse:getEllipseAngle(point);
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
        self.startAngle = 0
        self.endAngle = 0
    end

    local b = EntityBuilder(active_widget():document())
    local c = self:getEllipse(self.center, self.majorPoint, self.minorRadius, self.startAngle, self.endAngle)
    b:appendEntity(c)
    b:execute()

    luaInterface:deleteEvent('mouseMove', self)
    luaInterface:deleteEvent('number', self)
    luaInterface:deleteEvent('point', self)
end

function EllipseOperations:close()
    if(not self.finished) then
        active_widget():tempEntities():removeEntity(self.ellipse)
        self.finished = true

        luaInterface:deleteEvent('mouseMove', self)
        luaInterface:deleteEvent('number', self)
        luaInterface:deleteEvent('point', self)
    end
end