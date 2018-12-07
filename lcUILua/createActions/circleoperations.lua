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
    message("CIRCLE", self.target_widget)
    message("Provide Center Point:", self.target_widget)
end

function CircleOperations:enterCenter(eventName, data)
    if(eventName == "point") then
        self.builder:setCenter(data["position"])
        message("Options: <Radius> , Diameter", self.target_widget)
        message("Provide Radius:", self.target_widget)
        cli_get_text(self.target_widget, true)
        self.step = "enterRadius"
    end
end

function CircleOperations:enterRadius(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setRadius(self.builder:center():distanceTo(data["position"]))
    elseif(eventName == "text") then
        if (tonumber(data["text"]) == nil) then
            if (string.lower(data["text"]) == "d" or string.lower(data["text"]) == "diameter") then
                self.builder:setRadius(self.builder:radius() / 2)
                message("Provide Diameter:", self.target_widget)
                self.step = "enterDiameter"
            else
                message("Invalid input:" .. data["text"] ,self.target_widget)
                message("Provide Radius:", self.target_widget)
            end
        else
            message(data["text"],self.target_widget)
            self.builder:setRadius(tonumber(data["text"]))
            self:createEntity()
        end
    end
    if(eventName == "point") then
        self:createEntity()
    end
end

function CircleOperations:enterDiameter(eventName, data)
    if(eventName == "point" or eventName == "mouseMove") then
        self.builder:setRadius(self.builder:center():distanceTo(data["position"]) / 2)
    elseif(eventName == "text") then
        if (tonumber(data["text"]) == nil) then
            message("Invalid input:" .. data["text"] ,self.target_widget)
            message("Provide Diameter:", self.target_widget)
        else
            message(data["text"],self.target_widget)
            self.builder:setRadius(tonumber(data["text"])/2)
            self:createEntity()
        end
    end
    if(eventName == "point") then
        self:createEntity()
    end
end