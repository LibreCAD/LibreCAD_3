function active_widget()
    return mdiArea:activeSubWindow():widget()
end

local function click()
    local position = active_widget():cursor():position()
    event.trigger('point', position)
end

function new_document()
    cadMdiChild = lc.CadMdiChild()
    cadMdiChild:newDocument()
    cadMdiChild:viewer():autoScale()
    cadMdiChild:setDestroyCallback(onMdiChildDestroyed)

    luaInterface:luaConnect(cadMdiChild:view(), "mousePressEvent()", click)
    luaInterface:connect(cadMdiChild, "keyPressed(QKeyEvent*)", cliCommand, "onKeyPressed(QKeyEvent*)")

    return cadMdiChild
end

function load_document(fileName)
    cadMdiChild = lc.CadMdiChild()
    cadMdiChild:import(fileName)
    cadMdiChild:viewer():autoScale()

    luaInterface:luaConnect(cadMdiChild:view(), "mousePressEvent()", click)
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