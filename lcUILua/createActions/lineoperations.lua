LineOperations = {}
LineOperations.__index = LineOperations

setmetatable(LineOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function LineOperations:_init(id)
    self.lastPoint = nil
    self.length = nil
    self.entity_id = ID():id()
    message("Click on first point", id)

    CreateOperations._init(self, id)
end

function LineOperations:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(eventName == "point") then
        self:newPoint(data["position"])
    elseif(eventName == "mouseMove") then
        self:createTempLine(data["position"])
    elseif(eventName == "number") then
        self.length = data["number"]
    end
end

function LineOperations:newPoint(point)
    if(self.lastPoint ~= nil) then
        local l = self:createLine(self.lastPoint, point)
        self:createEntity(l)
        CreateOperations.close(self)
    else
        self.lastPoint = point
        message("Click on second point or enter line length", self.target_widget)
    end
end

function LineOperations:createTempLine(point)
    if(self.lastPoint ~= nil) then
        self.entity = self:createLine(self.lastPoint, point)
        self:refreshTempEntity()
    end
end

function LineOperations:createLine(p1, p2)
    local layer = active_layer(self.target_widget)
    local metaInfo = active_metaInfo(self.target_widget)

    if(self.length ~= nil) then
        local angle = p1:angleTo(p2)
        local relativeCoordinate = Coordinate._fromAngle(angle):mulDouble(self.length)
        p2 = p1:add(relativeCoordinate)
    end

    local l = Line(p1, p2, layer, metaInfo)
    l:setId(self.entity_id)

    return l
end
