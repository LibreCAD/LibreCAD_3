local iconSize = qt.QSize(24, 24)

local function create_button(name, icon)
    local button = qt.QPushButton(qt.QString(name))
    button:setFlat(true)

    if(icon ~= nil) then
        button:setIcon(qt.QIcon(qt.QString(icon)))
        button:setIconSize(iconSize)
    end

    return button
end

local function create_action(menu, name, icon)
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
end