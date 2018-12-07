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
    Operations._init(self, id, lc.builder.SplineBuilder, "enterPoint")
    self.tempPoint = false
    message("Add a new points or enter degree", id)
end

function SplineOperations:enterPoint(eventName, data)
    if(eventName == "point") then
        self.builder:addControlPoint(data["position"])
        for v in self.builder:controlPoints() do
            print(v)
        end
    elseif(eventName == "mouseMove") then
        if(self.tempPoint) then
            self.builder:removeControlPoint(-1)
        end

        self.tempPoint = true
        self.builder:addControlPoint(data["position"])
    elseif(eventName == "number") then
        if(data["number"] >= 0 and data["number"] <= 3) then
            self.builder:setDegree(data["number"])
        end
    end
end