SplineOperations = {}
SplineOperations.__index = SplineOperations

setmetatable(SplineOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function SplineOperations:_init(id)


    self.points = {}

    self.entity_id = ID():id()
    self.degree = 3
    self.entity = nil

    self:registerEvents()

    Operations._init(self, id)

    message("Add a new points or enter degree", id)
end

function SplineOperations:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(eventName == "point") then
        self:newPoint(data["position"])
    elseif(eventName == "mouseMove") then
        self:createTempSpline(data["position"])
    elseif(eventName == "number") then
        if(data["number"] >= 0 and data["number"] <= 3) then
            self.degree = data["number"]
        end
    end
end

function SplineOperations:newPoint(point)
    table.insert(self.points, point)
end

function SplineOperations:getSpline(points, degree)
    if(#points <= degree) then
        return nil
    end

    local layer = active_layer(self.target_widget)
    local metaInfo = active_metaInfo(self.target_widget)
    local s = Spline(points, {}, {}, degree, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, layer, metaInfo)

    s:setId(self.entity_id)

    return s
end

function SplineOperations:createTempSpline(point)
    local points = {}
    for k, v in pairs(self.points) do
        points[k] = v
    end
    table.insert(points, point)

    self.entity = self:getSpline(points, self.degree)
    self:refreshTempEntity()
end

function SplineOperations:createSpline()
    local s = self:getSpline(self.points, self.degree)
    if(s ~= nil) then
        self:createEntity(s)
    end
end

function SplineOperations:close()
    if(not self.finished) then
        self:createSpline()

        CreateOperations.close(self)
    end
end
