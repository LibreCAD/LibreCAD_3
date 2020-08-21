TextOperations = {
    name = "TextOperations",
    command_line = "TEXT"
}
TextOperations.__index = TextOperations

setmetatable(TextOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function TextOperations:_init()
    CreateOperations._init(self, lc.builder.TextBuilder, "enterTextValue")
    message("Enter text value")
    self.builder:setTextValue("Text")
    mainWindow:cliCommand():commandActive(true)
end

function TextOperations:enterTextValue(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:setInsertionPoint(data["position"])
    elseif(eventName == "text") then
        self.builder:setTextValue(data["text"])
        self.step = "enterInsertionPoint"
        message("Enter insertion point")
    end
end

function TextOperations:enterInsertionPoint(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:setInsertionPoint(data["position"])
    elseif(eventName == "point") then
        self.builder:setInsertionPoint(data["position"])
        self.step = "enterHeight"
        message("Enter height of text")
    end
end

function TextOperations:enterHeight(eventName, data)
    if(eventName == "mouseMove") then
        local dist = self.builder:insertionPoint():distanceTo(data["position"])
        self.builder:setHeight(dist)
    elseif(eventName == "point") then
        local dist = self.builder:insertionPoint():distanceTo(data["position"])
        self.builder:setHeight(dist)
        self.step = "enterAngle"
    elseif(eventName == "number") then
        self.builder:setHeight(data["number"])
        self.step = "enterAngle"
        message("Enter angle of text (in degrees)")
    end
end

function TextOperations:enterAngle(eventName, data)
    if(eventName == "mouseMove") then
        local ang = self.builder:insertionPoint():distanceTo(data["position"]) * 0.05
        self.builder:setAngle(ang)
    elseif(eventName == "point") then
        local ang = self.builder:insertionPoint():distanceTo(data["position"]) * 0.05
        self.builder:setAngle(ang)
        self:createEntity()
    elseif(eventName == "number") then
        self.builder:setAngle(data["number"] * 3.1416/180)
        self:createEntity()
    end
end

function TextOperations:copyEntity(textEntity)
    self.builder:copy(textEntity)
end
