--Remove "Current operation" group in the toolbar
local function remove_operation_group(eventName, id)
    if(hideUI ~= true) then
        local quickAccessTab = get_tab(id, "Quick Access")
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
        local quickAccessTab = get_tab(id, "Quick Access")
        local operationGroup = quickAccessTab:addGroup("Current operation")

        local cancelButton = create_button("", ":/icons/quit.svg", "Cancel")
        quickAccessTab:addWidget(operationGroup, cancelButton, 0, 0, 1, 1)
        luaInterface:luaConnect(cancelButton, "pressed()", function() finish_operation(id) end)
    end
end

luaInterface:registerEvent('operationFinished', remove_operation_group)
luaInterface:registerEvent('finishOperation', finish)

--Every function corresponding to the buttons in the toolbar or commands in cli widget
function run_basic_operation(id, operation, init_method, ...)
    mainWindow:getCliCommand():setFocus()
    --focusClicommand(id)
    finish_operation(id)
    --create_cancel_button(id)
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
    --focusClicommand(id)
    mainWindow:getCliCommand():setFocus()
    finish_operation(id)
    create_cancel_button(id)

    if(hideUI ~= true) then
        local quickAccessTab = get_tab(id, "Quick Access")
        local group = quickAccessTab:groupByName("Current operation")

        local lineButton = create_button("", ":/icons/linesnormal.png", "Line")
        quickAccessTab:addWidget(group, lineButton, 0, 1, 1, 1)
        luaInterface:luaConnect(lineButton, "pressed()", function()
            luaInterface:operation(id):createLine()
        end)

        local arcButton = create_button("", ":/icons/arc.svg", "Arc")
        quickAccessTab:addWidget(group, arcButton, 1, 1, 1, 1)
        luaInterface:luaConnect(arcButton, "pressed()", function()
            luaInterface:operation(id):createArc()
        end)
    end

    luaInterface:setOperation(id, LWPolylineOperations(id))
end
