local iconSize = qt.QSize(24, 24)

function create_button(name, icon)
    local button = qt.QPushButton(qt.QString(name))
    button:setFlat(true)

    if(icon ~= nil) then
        button:setIcon(qt.QIcon(qt.QString(icon)))
        button:setIconSize(iconSize)
    end

    return button
end

function create_action(menu, name, icon)
    local action = menu:addActionStr(qt.QString(name))

    if(icon ~= nil) then
        action:setIcon(qt.QIcon(qt.QString(icon)))
    end

    return action
end

function add_toolbar()
    toolbar = lc.Toolbar(mainWindow)
    mainWindow:addDockWidget(4, toolbar)

    local quickAccess = lc.ToolbarTab()
    local creationGroup = quickAccess:addGroup("Creation")

    local lineButton = create_button("", ":/icons/linesnormal.png")
    quickAccess:addButton(creationGroup, lineButton, 0, 0, 1, 1)
    luaInterface:luaConnect(lineButton, "pressed()", create_line)

    local circleButton = create_button("", ":/icons/circle.svg")
    quickAccess:addButton(creationGroup, circleButton, 1, 0, 1, 1)
    luaInterface:luaConnect(circleButton, "pressed()", create_circle)

    local arcButton = create_button("", ":/icons/arc.svg")
    quickAccess:addButton(creationGroup, arcButton, 0, 1, 1, 1)
    luaInterface:luaConnect(arcButton, "pressed()", create_arc)

    local spline = create_button("", ":/icons/spline.svg")
    quickAccess:addButton(creationGroup, spline, 2, 0, 1, 1)
    luaInterface:luaConnect(spline, "pressed()", create_spline)

    local polylineButton = create_button("", ":/icons/polylines.svg")
    quickAccess:addButton(creationGroup, polylineButton, 2, 1, 1, 1)
    luaInterface:luaConnect(polylineButton, "pressed()", create_lw_polyline)


    --
    -- Ellipses
    --
    local ellipseButton = create_button("", ":/icons/ellipse.svg")
    local ellipseMenu = qt.QMenu()

    local ellipseAction = create_action(ellipseMenu, "Ellipse", ":/icons/ellipse_axis.svg")
    luaInterface:luaConnect(ellipseAction, "triggered(bool)", create_ellipse)

    local arcEllipseAction = create_action(ellipseMenu, "Arc Ellipse", ":/icons/ellipse_arc_axis.svg")
    luaInterface:luaConnect(arcEllipseAction, "triggered(bool)", create_arc_ellipse)

    ellipseButton:setMenu(ellipseMenu)
    quickAccess:addButton(creationGroup, ellipseButton, 1, 1, 1, 1)

    toolbar:addTab("Quick Access", quickAccess)

    --
    -- Dimensions
    --
    local dimGroup = quickAccess:addGroup("Dimensions")

    local dimAligned = create_button("", ":/icons/dim_aligned.svg")
    quickAccess:addButton(dimGroup, dimAligned, 0, 0, 1, 1)
    luaInterface:luaConnect(dimAligned, "pressed()", create_dim_aligned)

    local dimAngular = create_button("", ":/icons/dim_angular.svg")
    quickAccess:addButton(dimGroup, dimAngular, 1, 0, 1, 1)
    luaInterface:luaConnect(dimAngular, "pressed()", create_dim_angular)

    local dimDiametric = create_button("", ":/icons/dim_diametric.svg")
    quickAccess:addButton(dimGroup, dimDiametric, 0, 1, 1, 1)
    luaInterface:luaConnect(dimDiametric, "pressed()", create_dim_diametric)

    local dimLinear = create_button("", ":/icons/dim_linear.svg")
    quickAccess:addButton(dimGroup, dimLinear, 1, 1, 1, 1)
    luaInterface:luaConnect(dimLinear, "pressed()", create_dim_linear)

    local dimRadial = create_button("", ":/icons/dim_radial.svg")
    quickAccess:addButton(dimGroup, dimRadial, 2, 0, 1, 1)
    luaInterface:luaConnect(dimRadial, "pressed()", create_dim_radial)

    --
    -- Modify
    --
    local modifyGroup = quickAccess:addGroup("Modify")

    local moveButton = create_button("", ":/icons/modifymove.png")
    quickAccess:addButton(modifyGroup, moveButton, 0, 0, 1, 1)
    luaInterface:luaConnect(moveButton, "pressed()", move_selected_entities)

    local rotateButton = create_button("", ":/icons/modifyrotate.png")
    quickAccess:addButton(modifyGroup, rotateButton, 1, 0, 1, 1)
    luaInterface:luaConnect(rotateButton, "pressed()", rotate_selected_entities)

    local copyButton = create_button("", ":/icons/move_copy.svg")
    quickAccess:addButton(modifyGroup, copyButton, 0, 1, 1, 1)
    luaInterface:luaConnect(copyButton, "pressed()", copy_selected_entities)

    local scaleButton = create_button("", ":/icons/scale.png")
    quickAccess:addButton(modifyGroup, scaleButton, 1, 1, 1, 1)
    luaInterface:luaConnect(scaleButton, "pressed()", scale_selected_entities)

    local removeButton = create_button("", ":/icons/delete.svg")
    quickAccess:addButton(modifyGroup, removeButton, 2, 0, 1, 1)
    luaInterface:luaConnect(removeButton, "pressed()", remove_selected_entities)

    local removeButton = create_button("", ":/icons/modifytrim.png")
    quickAccess:addButton(modifyGroup, removeButton, 2, 1, 1, 1)
    luaInterface:luaConnect(removeButton, "pressed()", trim_entity)

    --
    -- MetaInfo
    --
    local metaInfoGroup = quickAccess:addGroup("Entity properties")

    quickAccess:addWidget(metaInfoGroup, linePatternSelect, 0, 0, 1, 1)
    quickAccess:addWidget(metaInfoGroup, lineWidthSelect, 0, 1, 1, 1)
    quickAccess:addWidget(metaInfoGroup, colorSelect, 0, 2, 1, 1)

end
