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
    CreateOperations._init(self, id, lc.builder.CircleBuilder, "enterCenter")

    self.builder:setRadius(10)

    message("Click on center", self.target_widget)
end

function CircleOperations:enterCenter(eventName, data)
    if(eventName == "point") then
        self.builder:setCenter(point)
        message("Click on second point or enter the radius", self.target_widget)

        self.step = "enterRadius"
    end

    --         elseif(eventName == "mouseMove") then
    --              self:createTempCircle(data["position"])
end

function CircleOperations:enterRadius(eventName, data)
    if(eventName == "point") then
        self.builder:setRadius(self.builder:center():distanceTo(point))
        self:createCircle()
    elseif(eventName == "number") then
        self:createEntity(self:build())
        CreateOperations.close(self)
    end
end


function CircleOperations:createTempCircle(point)
    if(self.step == 1) then
        self.builder:setRadius(self.builder:center():distanceTo(point))
        self.entity = self:build()
        self:refreshTempEntity()
    end
end