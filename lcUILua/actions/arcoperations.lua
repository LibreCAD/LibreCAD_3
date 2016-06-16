ArcOperations = {}
ArcOperations.__index = ArcOperations

setmetatable(ArcOperations, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function ArcOperations:_init(id)
    Operations._init(self, id)

    self.center = nil
    self.radius = nil
    self.beginAngle = nil
    self.endAngle = nil
    self.arc = nil
    self.arc_id = ID():id()

    event.register('point', self)

    message("Click on center")
end

function ArcOperations:onEvent(eventName, ...)
    if(Operations.forMe(self) == false) then
        return
    end

    if(eventName == "point" or eventName == "number") then
        self:newData(...)
    elseif(eventName == "mouseMove") then
        self:createTempArc(...)
    end
end

function ArcOperations:newData(point)
    if(self.center == nil) then
        self.center = point
        self.arc = self:getArc(point, 0, 0, 0)
        active_widget():tempEntities():addEntity(self.arc)

        event.register('mouseMove', self)
        event.register('number', self)

        message("Click on second point or enter the radius")
    elseif(self.radius == nil) then
        self.radius = Operations:getDistance(self.center, point)
        message("Click on start point or enter the start angle")
    elseif(self.beginAngle == nil) then
        self.beginAngle = Operations:getAngle(self.center, point)
        message("Click on end point or enter the end angle")
    else
        self.endAngle = Operations:getAngle(self.center, point)
        self:createArc()
    end
end

function ArcOperations:getArc(center, radius, beginAngle, endAngle)
    local d = active_widget():document()
    local layer = d:layerByName("0")
    local a = Arc(center, radius, beginAngle, endAngle, false, layer)
    a:setId(self.arc_id)

    return a
end

function ArcOperations:createTempArc(point)
    local center = self.center
    local radius = self.radius
    local beginAngle = self.beginAngle
    local endAngle = self.endAngle

    if(center == nil) then
        center = point
    elseif(radius == nil) then
        radius = Operations:getDistance(center, point)
    elseif(beginAngle == nil) then
        beginAngle = Operations:getAngle(center, point)
    elseif(endAngle == nil) then
        endAngle = Operations:getAngle(center, point)
    end

    radius = radius or 0
    beginAngle = beginAngle or 0
    endAngle = endAngle or 0.001

    active_widget():tempEntities():removeEntity(self.arc)

    self.arc = self:getArc(center, radius, beginAngle, endAngle)

    active_widget():tempEntities():addEntity(self.arc)
end

function ArcOperations:createArc()
    active_widget():tempEntities():removeEntity(self.arc)

    local b = Builder(active_widget():document())
    local a = self:getArc(self.center, self.radius, self.beginAngle, self.endAngle)
    b:append(a)
    b:execute()

    event.delete('mouseMove', self)
    event.delete('number', self)
    event.delete('point', self)
end