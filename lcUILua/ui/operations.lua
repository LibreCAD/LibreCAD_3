--Remove "Current operation" group in the toolbar
local function remove_operation_group(eventName, id)
    if(hideUI ~= true) then
        local quickAccessTab = mainWindow:getToolbar():tabByName("Quick Access")
        local group = quickAccessTab:groupByName("Current operation")
        if(group ~= nil) then
            quickAccessTab:removeGroup(group)
        end
    end

    getWindow(id):viewer():setOperationActive(false)
end

function finish(eventName, id)
    finish_operation(id)
end

--End the current operation, even if it's not finished
function finish_operation(id)
    local op = luaInterface:operation(id);
    if(op ~= nil) then
        op:close()
    end
end

--Add a cancel button in the toolbar for the current operation
local function create_cancel_button(id)
    if(hideUI ~= true) then
        local quickAccessTab = mainWindow:getToolbar():tabByName("Quick Access")
        local operationGroup = quickAccessTab:addGroup("Current operation")

        mainWindow:getToolbar():addButton("", ":/icons/quit.svg", operationGroup, 0, 0, function() finish_operation(id) end, "Cancel")
    end
end

luaInterface:registerEvent('operationFinished', remove_operation_group)
luaInterface:registerEvent('finishOperation', finish)

--Every function corresponding to the buttons in the toolbar or commands in cli widget
function run_basic_operation(id, operation, init_method, ...)
    mainWindow:getCliCommand():setFocus()
    finish_operation(id)
    create_cancel_button(id)
    luaInterface:setOperation(id, operation(id, ...))
	op = luaInterface:operation(id)
    if(init_method) then
        op[init_method](op, ...)
    else
        if(op['_init_default'] ~= nil) then
           op['_init_default'](op)
        end
    end
	return op
end

function create_lw_polyline(id)
    mainWindow:getCliCommand():setFocus()
    finish_operation(id)
    create_cancel_button(id)

    if(hideUI ~= true) then
        local toolbar = mainWindow:getToolbar()
        local quickAccessTab = toolbar:tabByName("Quick Access")
        local group = quickAccessTab:groupByName("Current operation")

        toolbar:addButton("", ":/icons/linesnormal.png", group, 0, 1, function()
            luaInterface:operation(id):createLine()
        end, "Line")

        toolbar:addButton("", ":/icons/arc.svg", group, 1, 1, function()
            luaInterface:operation(id):createArc()
        end, "Arc")
    end

    luaInterface:setOperation(id, LWPolylineOperations(id))
end
