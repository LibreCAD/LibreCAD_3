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

    toolbar:addTab("Quick Access", quickAccess)
end