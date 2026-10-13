-- Edit > Paste, Ctrl+V and the context menu's Paste start this through
-- MainWindow::pasteEvent: the clipboard's lower left corner follows the mouse,
-- and a click adds the copies there as one undo step.
PasteOperation = {
    name = "PasteOperation",
    command_line = "PASTE"
}
PasteOperation.__index = PasteOperation

setmetatable(PasteOperation, {
    __index = Operations,
    __call = function (o, ...)
        local self = setmetatable({}, o)
        self:_init(...)
        return self
    end,
})

function PasteOperation:_init()
    Operations._init(self)

    self.entities = mainWindow:clipboardEntities()
    self.tempEntities = {}

    if(#self.entities > 0) then
        self.basePoint = mainWindow:clipboardBasePoint()

        luaInterface:registerEvent('point', self)
        luaInterface:registerEvent('mouseMove', self)

        message("Click on insertion point")
    else
        message("Nothing to paste")
        self.finished = true
        luaInterface:triggerEvent('operationFinished')
    end
end

function PasteOperation:onEvent(eventName, event)
    if(eventName == "point") then
        mainWindow:pasteClipboard(event["position"]:sub(self.basePoint))
        self:close()
    elseif(eventName == "mouseMove") then
        self:preview(event["position"]:sub(self.basePoint))
    end
end

function PasteOperation:removePreview()
    local window = mainWindow:cadMdiChild()
    for k, entity in pairs(self.tempEntities) do
        window:tempEntities():removeEntity(entity)
    end
    self.tempEntities = {}
end

function PasteOperation:preview(offset)
    self:removePreview()

    local window = mainWindow:cadMdiChild()
    for k, entity in pairs(self.entities) do
        local newEntity = entity:copy(offset)
        window:tempEntities():addEntity(newEntity)
        table.insert(self.tempEntities, newEntity)
    end
end

function PasteOperation:close()
    if(not self.finished) then
        self.finished = true
        self:removePreview()

        luaInterface:deleteEvent('mouseMove', self)
        luaInterface:deleteEvent('point', self)

        luaInterface:triggerEvent('operationFinished')
    end
end
