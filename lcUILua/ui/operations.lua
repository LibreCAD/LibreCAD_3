local function remove_operation_group()
    local tab = toolbar:tabByName("Quick Access")
    local group = tab:groupByName("Current operation")
    if(group ~= nil) then
        tab:removeGroup(group)
    end

    active_widget():viewer():setOperationActive(false)
end

local function finish_operation()
    if(op[active_widget().id] ~= nil) then
        op[active_widget().id]:close()
    end
end

local function create_cancel_button()
    local tab = toolbar:tabByName("Quick Access")
    local operationGroup = tab:addGroup("Current operation")

    local cancelButton = create_button("", ":/icons/quit.svg")
    tab:addButton(operationGroup, cancelButton, 0, 0, 1, 1)
    luaInterface:luaConnect(cancelButton, "pressed()", finish_operation)
end

local function new_operation()
    finish_operation()
    create_cancel_button()

    active_widget():viewer():setOperationActive(true)
end

event.register("operationFinished", remove_operation_group)

function create_line()
    finish_operation()
    op[active_widget().id] = LineOperations(active_widget().id)
end

function create_circle()
    finish_operation()
    op[active_widget().id] = CircleOperations(active_widget().id)
end

function create_arc()
    finish_operation()
    op[active_widget().id] = ArcOperations(active_widget().id)
end

function create_ellipse()
    finish_operation()
    op[active_widget().id] = EllipseOperations(active_widget().id, false)
end

function create_arc_ellipse()
    finish_operation()
    op[active_widget().id] = EllipseOperations(active_widget().id, true)
end

function create_dim_aligned()
    finish_operation()
    op[active_widget().id] = DimAlignedOperations(active_widget().id)
end

function create_dim_angular()
    finish_operation()
    op[active_widget().id] = DimAngularOperations(active_widget().id)
end

function create_dim_diametric()
    finish_operation()
    op[active_widget().id] = DimDiametricOperations(active_widget().id)
end

function create_dim_linear()
    finish_operation()
    op[active_widget().id] = DimLinearOperations(active_widget().id)
end

function create_dim_radial()
    finish_operation()
    op[active_widget().id] = DimRadialOperations(active_widget().id)
end

function create_spline()
    finish_operation()
    create_cancel_button()
    op[active_widget().id] = SplineOperations(active_widget().id)
end

function create_lw_polyline()
    finish_operation()
    create_cancel_button()

    local tab = toolbar:tabByName("Quick Access")
    local group = tab:groupByName("Current operation")

    local lineButton = create_button("", ":/icons/linesnormal.png")
    tab:addButton(group, lineButton, 0, 1, 1, 1)
    luaInterface:luaConnect(lineButton, "pressed()", function()
        op[active_widget().id]:createLine()
    end)

    local arcButton = create_button("", ":/icons/arc.svg")
    tab:addButton(group, arcButton, 1, 1, 1, 1)
    luaInterface:luaConnect(arcButton, "pressed()", function()
        op[active_widget().id]:createArc()
    end)

    op[active_widget().id] = LWPolylineOperations(active_widget().id)
end

function move_selected_entities()
    new_operation()
    op[active_widget().id] = MoveOperation(active_widget().id)
end