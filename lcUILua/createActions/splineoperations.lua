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
    CreateOperations._init(self, id, lc.builder.SplineBuilder, "enterPoint")
    self.tempPoint = false
    message("Add a new points or enter degree", id)
end

function SplineOperations:enterPoint(eventName, data)
    if(self.tempPoint) then
        self.builder:removeControlPoint(-1)
        self.tempPoint = false
    end
    if(eventName == "mouseMove") then
        self.builder:addControlPoint(data["position"])
        self.tempPoint = true
    elseif(eventName == "point") then
        self.builder:addControlPoint(data["position"])
    elseif(eventName == "number") then
        if(data["number"] >= 0 and data["number"] <= 3) then
            self.builder:setDegree(data["number"])
        end
    end
end

function SplineOperations:refreshTempEntity()
    if(#self.builder:controlPoints() > 2) then
        CreateOperations.refreshTempEntity(self)
    end
end

function SplineOperations:close()
    if(self.tempPoint) then
            self.builder:removeControlPoint(-1)
            self.tempPoint = false
    end

    if(self.creatingSpline == nil) then
        self.creatingSpline = true -- Used to prevent looping
        self:createEntity()
        CreateOperations.close(self)
    end
end