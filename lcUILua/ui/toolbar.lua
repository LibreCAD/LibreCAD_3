local iconSize = qt.QSize(24, 24)
toolbars = {}

--Create a new QPushbutton with name and icon
function create_button(name, icon)
    local button = qt.QPushButton(qt.QString(name))
    button:setFlat(true)

    if(icon ~= nil) then
        button:setIcon(qt.QIcon(qt.QString(icon)))
        button:setIconSize(iconSize)
    end

    return button
end

--Create a new action in given menu with name and icon
function create_action(menu, name, icon)
    local action = menu:addActionStr(qt.QString(name))

    if(icon ~= nil) then
        action:setIcon(qt.QIcon(qt.QString(icon)))
    end

    return action
end

function create_tab(id, name)
    local tab = lc.ToolbarTab()

    toolbars[id]["tabs"][name] = tab
    toolbars[id]["widget"]:addTab(name, tab)

    return tab
end

function get_tab(id, name)
    if(toolbars[id] ~= nil) then
        return toolbars[id]["tabs"][name]
    end

    return nil
end

--Add toolbar to main window
function add_toolbar(mainWindow, id, linePatternSelect, lineWidthSelect, colorSelect)
    local toolbar = lc.Toolbar(mainWindow)
    toolbars[id] = {
        widget = toolbar,
        tabs = {}
    }

    mainWindow:addDockWidget(4, toolbar)

    local quickAccessTab = create_tab(id, "Quick Access")
    local creationGroup = quickAccessTab:addGroup("Creation")

    local lineButton = create_button("", ":/icons/linesnormal.png")
    quickAccessTab:addWidget(creationGroup, lineButton, 0, 0, 1, 1)
    luaInterface:luaConnect(lineButton, "pressed()", function() run_basic_operation(id, LineOperations) end)

    local circleButton = create_button("", ":/icons/circle.svg")
    quickAccessTab:addWidget(creationGroup, circleButton, 1, 0, 1, 1)
    luaInterface:luaConnect(circleButton, "pressed()", function() run_basic_operation(id, CircleOperations) end)

    local arcButton = create_button("", ":/icons/arc.svg")
    quickAccessTab:addWidget(creationGroup, arcButton, 0, 1, 1, 1)
    luaInterface:luaConnect(arcButton, "pressed()", function() run_basic_operation(id, ArcOperations) end)

    local spline = create_button("", ":/icons/spline.svg")
    quickAccessTab:addWidget(creationGroup, spline, 2, 0, 1, 1)
    luaInterface:luaConnect(spline, "pressed()", function() run_basic_operation(id, SplineOperations) end)

    local polylineButton = create_button("", ":/icons/polylines.svg")
    quickAccessTab:addWidget(creationGroup, polylineButton, 2, 1, 1, 1)
    luaInterface:luaConnect(polylineButton, "pressed()", function() create_lw_polyline(id) end)


    --
    -- Ellipses
    --
    local ellipseButton = create_button("", ":/icons/ellipse.svg")
    local ellipseMenu = qt.QMenu()

    local ellipseAction = create_action(ellipseMenu, "Ellipse", ":/icons/ellipse_axis.svg")
    luaInterface:luaConnect(ellipseAction, "triggered(bool)", function() run_basic_operation(id, EllipseOperations) end)

    local arcEllipseAction = create_action(ellipseMenu, "Arc Ellipse", ":/icons/ellipse_arc_axis.svg")
    luaInterface:luaConnect(arcEllipseAction, "triggered(bool)", function() run_basic_operation(id, EllipseOperations, true) end)

    ellipseButton:setMenu(ellipseMenu)
    quickAccessTab:addWidget(creationGroup, ellipseButton, 1, 1, 1, 1)
    --
    -- Dimensions
    --
    local dimGroup = quickAccessTab:addGroup("Dimensions")

    local dimAligned = create_button("", ":/icons/dim_aligned.svg")
    quickAccessTab:addWidget(dimGroup, dimAligned, 0, 0, 1, 1)
    luaInterface:luaConnect(dimAligned, "pressed()", function() run_basic_operation(id, DimAlignedOperations) end)

    local dimAngular = create_button("", ":/icons/dim_angular.svg")
    quickAccessTab:addWidget(dimGroup, dimAngular, 1, 0, 1, 1)
    luaInterface:luaConnect(dimAngular, "pressed()", function() run_basic_operation(id, DimAngularOperations) end)

    local dimDiametric = create_button("", ":/icons/dim_diametric.svg")
    quickAccessTab:addWidget(dimGroup, dimDiametric, 0, 1, 1, 1)
    luaInterface:luaConnect(dimDiametric, "pressed()", function() run_basic_operation(id, DimDiametricOperations) end)

    local dimLinear = create_button("", ":/icons/dim_linear.svg")
    quickAccessTab:addWidget(dimGroup, dimLinear, 1, 1, 1, 1)
    luaInterface:luaConnect(dimLinear, "pressed()", function() run_basic_operation(id, DimLinearOperations) end)

    local dimRadial = create_button("", ":/icons/dim_radial.svg")
    quickAccessTab:addWidget(dimGroup, dimRadial, 2, 0, 1, 1)
    luaInterface:luaConnect(dimRadial, "pressed()", function() run_basic_operation(id, DimRadialOperations) end)

    --
    -- Modify
    --
    local modifyGroup = quickAccessTab:addGroup("Modify")

    local moveButton = create_button("", ":/icons/modifymove.png")
    quickAccessTab:addWidget(modifyGroup, moveButton, 0, 0, 1, 1)
    luaInterface:luaConnect(moveButton, "pressed()", function() run_basic_operation(id, MoveOperation) end)

    local rotateButton = create_button("", ":/icons/modifyrotate.png")
    quickAccessTab:addWidget(modifyGroup, rotateButton, 1, 0, 1, 1)
    luaInterface:luaConnect(rotateButton, "pressed()", function() run_basic_operation(id, RotateOperation) end)

    local copyButton = create_button("", ":/icons/move_copy.svg")
    quickAccessTab:addWidget(modifyGroup, copyButton, 0, 1, 1, 1)
    luaInterface:luaConnect(copyButton, "pressed()", function() run_basic_operation(id, CopyOperation) end)

    local scaleButton = create_button("", ":/icons/scale.png")
    quickAccessTab:addWidget(modifyGroup, scaleButton, 1, 1, 1, 1)
    luaInterface:luaConnect(scaleButton, "pressed()", function() run_basic_operation(id, ScaleOperation) end)

    local removeButton = create_button("", ":/icons/delete.svg")
    quickAccessTab:addWidget(modifyGroup, removeButton, 2, 0, 1, 1)
    luaInterface:luaConnect(removeButton, "pressed()", function() run_basic_operation(id, RemoveOperation) end)

    local removeButton = create_button("", ":/icons/modifytrim.png")
    quickAccessTab:addWidget(modifyGroup, removeButton, 2, 1, 1, 1)
    luaInterface:luaConnect(removeButton, "pressed()", function() run_basic_operation(id, TrimOperation) end)

    --
    -- Snap options
    --
    local snapOptionGroup = quickAccessTab:addGroup("Snap options")
    local snapGridButton = create_button("", ":/icons/snap_grid.svg")
    snapGridButton:setCheckable(true)
    quickAccessTab:addWidget(snapOptionGroup, snapGridButton, 0, 0, 1, 1)
    luaInterface:luaConnect(snapGridButton, "toggled(bool)", function(enabled)
        getWindow(id):getSnapManager():setGridSnappable(enabled)
    end)

    local snapIntersectionButton = create_button("", ":/icons/snap_intersection.svg")
    snapIntersectionButton:setCheckable(true)
    quickAccessTab:addWidget(snapOptionGroup, snapIntersectionButton, 0, 1, 1, 1)
    luaInterface:luaConnect(snapIntersectionButton, "toggled(bool)", function(enabled)
        getWindow(id):getSnapManager():setIntersectionSnappable(enabled)
    end)

    local snapMiddleButton = create_button("", ":/icons/snap_middle.svg")
    snapMiddleButton:setCheckable(true)
    quickAccessTab:addWidget(snapOptionGroup, snapMiddleButton, 1, 0, 1, 1)
    luaInterface:luaConnect(snapMiddleButton, "toggled(bool)", function(enabled)
        getWindow(id):getSnapManager():setMiddleSnappable(enabled)
    end)

    local snapEntityButton = create_button("", ":/icons/snap_entity.svg")
    snapEntityButton:setCheckable(true)
    quickAccessTab:addWidget(snapOptionGroup, snapEntityButton, 1, 1, 1, 1)
    luaInterface:luaConnect(snapEntityButton, "toggled(bool)", function(enabled)
        getWindow(id):getSnapManager():setEntitySnappable(enabled)
    end)

    --
    -- MetaInfo
    --
    local metaInfoGroup = quickAccessTab:addGroup("Entity properties")

    quickAccessTab:addWidget(metaInfoGroup, linePatternSelect, 0, 0, 1, 1)
    quickAccessTab:addWidget(metaInfoGroup, lineWidthSelect, 0, 1, 1, 1)
    quickAccessTab:addWidget(metaInfoGroup, colorSelect, 0, 2, 1, 1)

end
