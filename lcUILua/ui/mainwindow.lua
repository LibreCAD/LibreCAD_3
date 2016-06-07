op = {}

function new_file()
    cadMdiChild = new_document()

    window = mdiArea:addSubWindow(cadMdiChild)
    cadMdiChild:showMaximized()

    id = nextTableId(op)
    op[id] = Operations:new(id)
    window:widget().id = id
end

function open_file()
    local fileName = qt.QFileDialog.getOpenFileName(
        mainWindow,
        qt.QObject.tr("Open File"),
        qt.QString(""),
        qt.QObject.tr("dxf(*.dxf);;dwg(*.dwg)")
    )

    if(not fileName:isEmpty()) then
        cadMdiChild = load_document(fileName:toStdString())

        window = mdiArea:addSubWindow(cadMdiChild)
        cadMdiChild:showMaximized()

        id = nextTableId(op)
        op[id] = Operations:new(id)
        window:widget().id = id
    end
end

function create_main_window()
    mainWindow = qt.loadUi(ui_path .. "/mainwindow.ui")
    mainWindow:setWindowTitle(qt.QObject.tr("LibreCAD"));
    mainWindow:setUnifiedTitleAndToolBarOnMac(true);

    mdiArea = mainWindow:findChild("centralWidget"):findChild("mdiArea")

    mdiArea:setHorizontalScrollBarPolicy(0)
    mdiArea:setVerticalScrollBarPolicy(0)

    mainWindow:setCentralWidget(mdiArea)

    create_menu()

    luaScript = lc.LuaScript(mdiArea)
    mainWindow:addDockWidget(2, luaScript)

    add_commandline()

    mainWindow:show()

    new_file()
end

function create_menu()
    menuBar = mainWindow:menuBar()
    drawMenu = menuBar:addMenuStr(qt.QString("Draw"))
    lineAction = drawMenu:addActionStr(qt.QString("Line"))

    luaInterface:luaConnect(mainWindow:findChild("actionNew"), "triggered(bool)", new_file)
    luaInterface:luaConnect(mainWindow:findChild("actionOpen"), "triggered(bool)", open_file)
    luaInterface:luaConnect(mainWindow:findChild("actionUndo"), "triggered(bool)", undo)
    luaInterface:luaConnect(mainWindow:findChild("actionRedo"), "triggered(bool)", redo)
    luaInterface:luaConnect(lineAction, "triggered(bool)", create_line)
end

function nextTableId(table)
    count = 0
    for id, v in pairs(table) do
        count = count + 1
        if(v == nil) then
            return count
        end
    end

    return count
end