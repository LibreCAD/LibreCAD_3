local iconSize = qt.QSize(24, 24)

function add_toolbar()
    toolbar = lc.Toolbar(mainWindow)
    mainWindow:addDockWidget(4, toolbar)

    local quickAccess = lc.ToolbarTab()
    local testGroup = quickAccess:addGroup("Creation")

    local lineButton = qt.QPushButton(qt.QString(""))
    lineButton:setFlat(true)
    lineButton:setIcon(qt.QIcon(qt.QString(":/icons/linesnormal.png")))
    lineButton:setIconSize(iconSize)

    quickAccess:addButton(testGroup, lineButton,0,0,1,1)

    luaInterface:luaConnect(lineButton, "pressed()", create_line)

    toolbar:addTab("Quick Access", quickAccess)
end