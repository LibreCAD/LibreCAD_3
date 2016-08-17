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

-- Create a new window containing an empty document
function new_document()
    cadMdiChild = lc.CadMdiChild()
    cadMdiChild:newDocument()
    cadMdiChild:viewer():autoScale()
    cadMdiChild:setDestroyCallback(onMdiChildDestroyed)

    luaInterface:luaConnect(cadMdiChild:view(), "mousePressEvent()", click)
    luaInterface:luaConnect(cadMdiChild:view(), "mouseReleaseEvent()", mouseRelease)
    luaInterface:luaConnect(cadMdiChild:view(), "mouseMoveEvent()", mouseMove)
    luaInterface:connect(cadMdiChild, "keyPressed(QKeyEvent*)", cliCommand, "onKeyPressed(QKeyEvent*)")

    return cadMdiChild
end

-- Create a new window with an existing document
function load_document(fileName)
    cadMdiChild = lc.CadMdiChild()
    cadMdiChild:import(fileName)
    cadMdiChild:viewer():autoScale()

    luaInterface:luaConnect(cadMdiChild:view(), "mousePressEvent()", click)
    luaInterface:luaConnect(cadMdiChild:view(), "mouseReleaseEvent()", mouseRelease)
    luaInterface:luaConnect(cadMdiChild:view(), "mouseMoveEvent()", mouseMove)
    luaInterface:connect(cadMdiChild, "keyPressed(QKeyEvent*)", cliCommand, "onKeyPressed(QKeyEvent*)")

    return cadMdiChild
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