--[[windows = {}

function getWindow(id)
    return windows[id]
end

-- Add a new CadMdiChild

function addCadMdiChild(cadMdiChild, id, cliCommand)
    if(cadMdiChild == nil) then
        return
    end

    cadMdiChild:setDestroyCallback(onMdiChildDestroyed)

    windows[id] = cadMdiChild
    cadMdiChild.id = id
end

-- Clean Lua information of window when it's destroyed
function onMdiChildDestroyed(id)
    windows[id] = nil
end]]--
