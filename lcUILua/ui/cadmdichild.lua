-- Return current window widget
function active_widget()
    local subWindow = mdiArea:activeSubWindow()
    if(subWindow == nil) then
        return nil
    end

    return subWindow:widget()
end

-- Return current window document
function active_document()
    local widget = active_widget()

    if(widget == nil) then
        return nil
    end

    return active_widget():document()
end

-- Send a click event on the current window
local function click()
    local widget = active_widget()
    if(widget == nil) then
        return
    end

    local position = widget:cursor():position()
    event.trigger('point', position)
end

-- Send an event with new position of the cursor on the current window
local function mouseMove()
    local widget = active_widget()
    if(widget == nil) then
        return
    end

    local position = widget:cursor():position()
    event.trigger('mouseMove', position)
end

-- Event triggered when the mouse is released
local function mouseRelease()
    event.trigger('selectionChanged')
end

-- Add a new CadMdiChild
function addCadMdiChild(cadMdiChild)
    if(cadMdiChild == nil) then
        return
    end

    cadMdiChild:viewer():autoScale()
    cadMdiChild:setDestroyCallback(onMdiChildDestroyed)

    luaInterface:luaConnect(cadMdiChild:viewer(), "mousePressEvent()", click)
    luaInterface:luaConnect(cadMdiChild:viewer(), "mouseReleaseEvent()", mouseRelease)
    luaInterface:luaConnect(cadMdiChild:viewer(), "mouseMoveEvent()", mouseMove)
    luaInterface:connect(cadMdiChild, "keyPressed(QKeyEvent*)", cliCommand, "onKeyPressed(QKeyEvent*)")

    mdiArea:addSubWindow(cadMdiChild)
    cadMdiChild:showMaximized()

    local id = nextTableId(op)
    op[id] = Operations(id)
    cadMdiChild.id = id
end

-- Undo last operation of the current window
function undo()
    if(active_widget() ~= nil) then
        active_widget():undoManager():undo()
    end
end

-- Redo last operation of the current window
function redo()
    if(active_widget() ~= nil) then
        active_widget():undoManager():redo()
    end
end

-- Clean Lua informations of window when it's destroyed
function onMdiChildDestroyed(id)
    op[id] = nil
end