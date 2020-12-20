MTextOperations = {
    name = "MTextOperations",
    command_line = "MTEXT",
    context_transitions = {
        enterTextValue = {"enterInsertionPoint", "enterHeight", "enterAngle"},
        enterInsertionPoint = {"enterTextValue", "enterHeight", "enterAngle"},
        enterHeight = {"enterTextValue", "enterInsertionPoint", "enterAngle"},
        enterAngle = {"enterTextValue", "enterInsertionPoint", "enterHeight"}
    }
}
MTextOperations.__index = MTextOperations

setmetatable(MTextOperations, {
    __index = CreateOperations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function MTextOperations:_init()
    CreateOperations._init(self, lc.builder.MTextBuilder, "enterTextValue")
    message("Enter mtext value")
    self.builder:setTextValue("MText")
    mainWindow:cliCommand():commandActive(true)
end

function MTextOperations:enterTextValue(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:setInsertionPoint(data["position"])
    elseif(eventName == "text") then
        self.builder:setTextValue(data["text"])
        self.textValue = data["text"]
        self:determineNextStep()
    end
end

function MTextOperations:enterInsertionPoint(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:setInsertionPoint(data["position"])
    elseif(eventName == "point") then
        self.builder:setInsertionPoint(data["position"])
        if self.byTextDialog ~= nil and self.byTextDialog then
            self:createEntity()
        else
            self.insertionPoint = data["position"]
            self:determineNextStep()
        end
    end
end

function MTextOperations:enterHeight(eventName, data)
    if(eventName == "mouseMove") then
        local dist = self.builder:insertionPoint():distanceTo(data["position"])
        self.builder:setHeight(dist)
    elseif(eventName == "point") then
        local dist = self.builder:insertionPoint():distanceTo(data["position"])
        self.builder:setHeight(dist)
        self.height = dist
        self:determineNextStep()
    elseif(eventName == "number") then
        self.builder:setHeight(data["number"])
        self.height = data["number"]
        self:determineNextStep()
    end
end

function MTextOperations:enterAngle(eventName, data)
    if(eventName == "mouseMove") then
        local ang = self.builder:insertionPoint():distanceTo(data["position"]) * 0.05
        self.builder:setAngle(ang)
    elseif(eventName == "point") then
        local ang = self.builder:insertionPoint():distanceTo(data["position"]) * 0.05
        self.builder:setAngle(ang)
        self.angle = ang
        self:determineNextStep()
    elseif(eventName == "number") then
        self.builder:setAngle(data["number"] * 3.1416/180)
        self.angle = self.builder:angle()
        self:determineNextStep()
    end
end

function MTextOperations:copyEntity(textEntity)
    self.builder:copy(textEntity)
    self.byTextDialog = true
    self.step = "enterInsertionPoint"
    message("Enter insertion point")
end

function MTextOperations:determineNextStep()
    if(self.textValue == nil) then
        message("Enter text value")
        self.step = "enterTextValue"
    elseif(self.insertionPoint == nil) then
        message("Enter insertion point")
        self.step = "enterInsertionPoint"
    elseif(self.height == nil) then
        message("Enter height of text")
        self.step = "enterHeight"
    elseif(self.angle == nil) then
        message("Enter angle of text (in degrees)")
        self.step = "enterAngle"
    else
        message("Done")
        self:createEntity()
    end
end
