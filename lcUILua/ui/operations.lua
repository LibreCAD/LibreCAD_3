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

        local cancelButton = create_button("", ":/icons/quit.svg")
        quickAccessTab:addWidget(operationGroup, cancelButton, 0, 0, 1, 1)
        luaInterface:luaConnect(cancelButton, "pressed()", finish_operation)
    end
end

luaInterface:registerEvent('operationFinished', remove_operation_group)

--Every function corresponding to the buttons in the toolbar or commands in cli widget
function run_basic_operation(id, operation)
    finish_operation(id)
    create_cancel_button(id)
    luaInterface:setOperation(id, operation(id))
end

function create_lw_polyline(id)
    finish_operation(id)
    create_cancel_button(id)

    if(hideUI ~= true) then
        local group = quickAccessTab:groupByName("Current operation")

        local lineButton = create_button("", ":/icons/linesnormal.png")
        quickAccessTab:addWidget(group, lineButton, 0, 1, 1, 1)
        luaInterface:luaConnect(lineButton, "pressed()", function()
            luaInterface:operation(id):createLine()
        end)

        local arcButton = create_button("", ":/icons/arc.svg")
        quickAccessTab:addWidget(group, arcButton, 1, 1, 1, 1)
        luaInterface:luaConnect(arcButton, "pressed()", function()
            luaInterface:operation(id):createArc()
        end)
    end

    luaInterface:setOperation(id, LWPolylineOperations(id))
end

function snapable_options(enabled)
    gridSnapableOptions(enabled)
end