windows = {}

function getWindow(id)
    return windows[id]
end

-- Add a new CadMdiChild

function addCadMdiChild(cadMdiChild, id, cliCommand)
    if(cadMdiChild == nil) then
        return
    end

    cadMdiChild:viewer():autoScale()
    cadMdiChild:setDestroyCallback(onMdiChildDestroyed)

    windows[id] = cadMdiChild
    cadMdiChild.id = id

    luaInterface:luaConnect(cadMdiChild:viewerProxy(), "mousePressEvent()", function()
        local position = cadMdiChild:cursor():position()
        print(position:x() .. " " .. position:y())
        luaInterface:triggerEvent('point', {position = position, widget = cadMdiChild})
    end)

    luaInterface:luaConnect(cadMdiChild:viewerProxy(), "mouseReleaseEvent()", function()
        luaInterface:triggerEvent('selectionChanged', {widget = cadMdiChild})
    end)

    luaInterface:luaConnect(cadMdiChild:viewerProxy(), "mouseMoveEvent()", function()
        local position = cadMdiChild:cursor():position()
        luaInterface:triggerEvent('mouseMove', {position = position, widget = cadMdiChild})
    end)

    luaInterface:luaConnect(cadMdiChild:viewerProxy(), "keyPressEvent(int)", function(key)
        --Here int has ascii value
        if (key==16777216) then
            --If Escape key is pressed
            finish_operation(id)
        else
            luaInterface:triggerEvent('keyPressed', {key = key, widget = cadMdiChild})
        end
    end)

    luaInterface:connect(cadMdiChild, "keyPressed(QKeyEvent*)", cliCommand, "onKeyPressed(QKeyEvent*)")
end

-- Clean Lua information of window when it's destroyed
function onMdiChildDestroyed(id)
    windows[id] = nil
end

luaInterface:registerEvent('point', function(e, data)
    data["widget"]:viewer():docCanvas():selectPoint(data["position"]:x(), data["position"]:y())
end)