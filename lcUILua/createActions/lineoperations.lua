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
    self.builder = lc.builder.LineBuilder()
    self.step = 0
    self.length = nil
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
        self.length = data
    end
end

function LineOperations:newPoint(point)
    if(self.step == 0) then
        self.builder:setStartPoint(point)
        message("Click on second point or enter line length")

        self.step = self.step + 1
    else
        self:createEntity(self:build())
        CreateOperations.close(self)
    end
end

function LineOperations:setEndPoint(point)
    local angle = self.builder:endPoint():angleTo(p2)
    local relativeCoordinate = Coordinate._fromAngle(angle):mulDouble(self.length)
    self.builder:setEndPoint(p1:add(relativeCoordinate))
end

function LineOperations:createTempLine(point)
    if(self.step ~= 0) then
        self.builder:setEndPoint(point)
        self.entity = self:build()
        self:refreshTempEntity()
    end
end
