function active_widget()
    local subWindow = mdiArea:activeSubWindow()
    if(subWindow == nil) then
        return nil
    end

    return subWindow:widget()
end

function active_document()
    local widget = active_widget()

    if(widget == nil) then
        return nil
    end

    return active_widget():document()
end

local function click()
    local widget = active_widget()
    if(widget == nil) then
        return
    end

    local position = widget:cursor():position()
    event.trigger('point', position)
end

local function mouseMove()
    local widget = active_widget()
    if(widget == nil) then
        return
    end

    local position = widget:cursor():position()
    event.trigger('mouseMove', position)
end

local function mouseRelease()
    event.trigger('selectionChanged')
end

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

function undo()
    if(active_widget() ~= nil) then
        active_widget():undoManager():undo()
    end
end

function redo()
    if(active_widget() ~= nil) then
        active_widget():undoManager():redo()
    end
end



function onMdiChildDestroyed(id)
    op[id] = nil
end