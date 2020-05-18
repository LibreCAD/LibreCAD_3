--Write a message to the command line log
--[[function message(message)
    local cliCommand = mainWindow:getCliCommand()
    if cliCommand ~= nil then
        cliCommand:write(tostring(message))
    end
end

--Remove "Current operation" group in the toolbar
--[[local function remove_operation_group(eventName)
    if(hideUI ~= true) then
        mainWindow:getToolbar():removeGroupByName("Current operation")
    end

    mainWindow:getCadMdiChild():viewer():setOperationActive(false)
end]]--

--[[function finish(eventName)
    print("FINISH CALLED")
    finish_operation()
end

--End the current operation, even if it's not finished
function finish_operation()
    local op = luaInterface:operation();
    if(op ~= nil) then
        op:close()
    end
    print("FINISH OPERATION CALLED")
end]]--

--Add a cancel button in the toolbar for the current operation
--[[local function create_cancel_button()
    if(hideUI ~= true) then
        mainWindow:getToolbar():addButton("", ":/icons/quit.svg", "Current operation", 0, 0, function() finish_operation() end, "Cancel")
    end
end

--luaInterface:registerEvent('operationFinished', remove_operation_group)
--luaInterface:registerEvent('finishOperation', finish)

--Every function corresponding to the buttons in the toolbar or commands in cli widget
--[[function run_basic_operation(operation, init_method, ...)
    mainWindow:getCliCommand():setFocus()
    finish_operation()
    create_cancel_button()
    luaInterface:setOperation(operation(...))
	op = luaInterface:operation()
    if(init_method) then
        op[init_method](op, ...)
    else
        if(op['_init_default'] ~= nil) then
           op['_init_default'](op)
        end
    end
	return op
end]]--

function create_lw_polyline()
    mainWindow:getCliCommand():setFocus()
    finish_operation()
    create_cancel_button()

    if(hideUI ~= true) then
        local toolbar = mainWindow:getToolbar()
        toolbar:addButton("", ":/icons/linesnormal.png", "Current operation", 0, 1, function()
            luaInterface:operation():createLine()
        end, "Line")

        toolbar:addButton("", ":/icons/arc.svg", "Current operation", 1, 1, function()
            luaInterface:operation():createArc()
        end, "Arc")
    end

    luaInterface:setOperation(LWPolylineOperations())
end

--
-- Toolbar Snap options
--
mainWindow:getToolbar():addCheckableButton("", ":/icons/snap_grid.svg", "Snap Options", 0, 0, function(enabled)
    mainWindow:getCadMdiChild():getSnapManager():setGridSnappable(enabled)
end, "Snap Grid")
mainWindow:getToolbar():addCheckableButton("", ":/icons/snap_intersection.svg", "Snap Options", 0, 1, function(enabled)
    mainWindow:getCadMdiChild():getSnapManager():setIntersectionSnappable(enabled)
end, "Snap Intersection")
mainWindow:getToolbar():addCheckableButton("", ":/icons/snap_middle.svg", "Snap Options", 1, 0, function(enabled)
    mainWindow:getCadMdiChild():getSnapManager():setMiddleSnappable(enabled)
end, "Snap Middle")
mainWindow:getToolbar():addCheckableButton("", ":/icons/snap_entity.svg", "Snap Options", 1, 1, function(enabled)
    mainWindow:getCadMdiChild():getSnapManager():setEntitySnappable(enabled)
end, "Snap Entity")
