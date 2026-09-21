-- The MText creation operation.
--
-- Issue #361 asks whether this operation is worth keeping, since it was a
-- verbatim copy of textoperations.lua and "if mtext is only to be created by
-- the text dialog (and not the commandline or tool) then we can simplify the
-- lua operation".  It is kept, for two reasons:
--
--   * The dialog does not replace it.  TextDialog::okButtonClicked starts
--     THIS operation (runOperationByName("MTextOperations")) and then hands it
--     the built entity via copyEntity, so the operation is what places the
--     insertion point on the dialog route too.  Simplifying it away would
--     break the dialog, not just the command line.
--   * MTEXT on the command line was the only route to an MText that does not
--     open a modal dialog, and it had no toolbar button and no menu entry.
--     Both are added here rather than removed.
--
-- What makes it an *MText* operation and not a copy of the text one is the
-- line breaks: see decodeTypedText.

MTextOperations = {
    name = "MTextOperations",
    command_line = "MTEXT",
    icon = "mtext.svg",
    description = "Multiline Text",
    menu_actions = {
        default = "actionMText_Operation"
    },
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

-- The escapes a typed MText value understands, keyed by the character after
-- the backslash.
local TYPED_ESCAPES = {
    ["P"] = "\n",         -- paragraph break
    ["\\"] = "\\",        -- a literal backslash
    ["{"] = "{",
    ["}"] = "}",
    ["~"] = "\194\160",   -- U+00A0 no-break space
}

--- Turn a value typed at the command line into the text of a multi-line MText.
--
-- The command line is one line, so without this an MText created there can
-- only ever have one line -- which is the whole difference between MText and
-- Text.  The spelling is not invented for the command line: `\P` is what a
-- paragraph break is called in the file format, and `\\` is how that format
-- writes one literal backslash, so what you type here is what
-- persistence/libdxfrw/mtextcodec.h reads back out of a DXF.
--
-- Only the escapes above are recognised.  A backslash before anything else is
-- kept as typed: dropping characters the user can see is worse than leaving a
-- rare escape alone.
function MTextOperations.decodeTypedText(text)
    if type(text) ~= "string" then
        return text
    end

    local out = {}
    local i = 1
    local n = #text

    while i <= n do
        local c = text:sub(i, i)
        local escape = nil

        if c == "\\" then
            escape = TYPED_ESCAPES[text:sub(i + 1, i + 1)]
        end

        if escape ~= nil then
            out[#out + 1] = escape
            i = i + 2
        else
            out[#out + 1] = c
            i = i + 1
        end
    end

    return table.concat(out)
end

function MTextOperations:_init()
    CreateOperations._init(self, lc.builder.MTextBuilder, "enterTextValue")
    message("Enter mtext value (\\P starts a new line)")
    self.builder:setTextValue("MText")
    mainWindow:cliCommand():commandActive(true)
end

function MTextOperations:enterTextValue(eventName, data)
    if(eventName == "mouseMove") then
        self.builder:setInsertionPoint(data["position"])
    elseif(eventName == "text") then
        local value = MTextOperations.decodeTypedText(data["text"])
        self.builder:setTextValue(value)
        self.textValue = value
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
        message("Enter mtext value (\\P starts a new line)")
        self.step = "enterTextValue"
    elseif(self.insertionPoint == nil) then
        message("Enter insertion point")
        self.step = "enterInsertionPoint"
    elseif(self.height == nil) then
        message("Enter height of mtext")
        self.step = "enterHeight"
    elseif(self.angle == nil) then
        message("Enter angle of mtext (in degrees)")
        self.step = "enterAngle"
    else
        message("Done")
        self:createEntity()
    end
end
