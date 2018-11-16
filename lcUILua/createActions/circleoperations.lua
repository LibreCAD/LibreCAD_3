CircleOperations = {}
CircleOperations.__index = CircleOperations

setmetatable(CircleOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function CircleOperations:_init(id)
    self.builder = lc.builder.CircleBuilder()
    self.builder:setRadius(10)
    self.step = 0

    CreateOperations._init(self, id)
    message("Click on center", self.target_widget)
end

function CircleOperations:onEvent(eventName, data)
    if(Operations.forMe(self, data) == false) then
        return
    end

    if(eventName == "point") then
        self:newPoint(data["position"])
    elseif(eventName == "mouseMove") then
        self:createTempCircle(data["position"])
    elseif(eventName == "number") then
        self:createCircle(data["number"])
    end
end

function CircleOperations:newPoint(point)
    if(self.step == 0) then
        self.builder:setCenter(point)

        message("Click on second point or enter the radius", self.target_widget)
        self.step = self.step + 1
    else
        self.builder:setRadius(self.builder:center():distanceTo(point))
        self:createCircle()
    end
end


function CircleOperations:createTempCircle(point)
    if(self.step == 1) then
        self.builder:setRadius(self.builder:center():distanceTo(point))
        self.entity = self:build()
        self:refreshTempEntity()
    end
end

function CircleOperations:createCircle()
    self:createEntity(self:build())

    CreateOperations.close(self)
end