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

local function open_lua_script()
    local luaScript = lc.LuaScript(mdiArea, cliCommand)
    luaScript:show()
end

local function create_menu()
    local menuBar = mainWindow:menuBar()
    local drawMenu = menuBar:addMenuStr(qt.QString("Draw"))
    local lineAction = drawMenu:addActionStr(qt.QString("Line"))

    local luaMenu = menuBar:addMenuStr(qt.QString("Lua"))
    local luaScriptAction = luaMenu:addActionStr(qt.QString("Run script"))

    luaInterface:luaConnect(mainWindow:findChild("actionNew"), "triggered(bool)", new_file)
    luaInterface:luaConnect(mainWindow:findChild("actionOpen"), "triggered(bool)", open_file)
    luaInterface:luaConnect(mainWindow:findChild("actionUndo"), "triggered(bool)", undo)
    luaInterface:luaConnect(mainWindow:findChild("actionRedo"), "triggered(bool)", redo)

    luaInterface:luaConnect(lineAction, "triggered(bool)", create_line)

    luaInterface:luaConnect(luaScriptAction, "triggered(bool)", open_lua_script)
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

    add_commandline()

    mainWindow:show()

    new_file()
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