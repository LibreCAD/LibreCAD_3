op = {}

--Create empty new window
function new_file()
    local cadMdiChild = lc.CadMdiChild()
    cadMdiChild:newDocument()

    create_new_window(cadMdiChild);
end

function open_file()
    -- @todo: It could be better if we create the CadMdiChild only if the filename is correct
    local cadMdiChild = lc.CadMdiChild()
    if(cadMdiChild:openFile()) then
        create_new_window(cadMdiChild)
    end
end

--Create save file dialog and save the file
function save_file(id)
    if(windows[id] == nil) then
        return
    end

    windows[id]:saveFile()
end

--Return the selected layer
function active_layer(id)
    if(windows[id] == nil) then
        return nil
    end

    return windows[id]:activeLayer()
end

--Create MetaInfo containing every selected MetaTypes
function active_metaInfo()
    local widget = windows[id]

    if(widget == nil) then
        return nil
    end

    local metaInfo = widget:metaInfoManager():metaInfo()
    if(metaInfo == nil) then
        metaInfo = MetaInfo() -- nil != nullptr when calling a c++ function
    end

    return metaInfo
end

--Create dialog to enter Lua script
local function open_lua_script()
    local luaScript = lc.LuaScript(mdiArea, cliCommand)
    luaScript:show()
end


--Create main window menu
local function create_menu(mainWindow, widget)
    local menuBar = mainWindow:menuBar()
    local drawMenu = menuBar:addMenuStr(qt.QString("Draw"))
    local lineAction = drawMenu:addActionStr(qt.QString("Line"))
    local circleAction = drawMenu:addActionStr(qt.QString("Circle"))
    local arcAction = drawMenu:addActionStr(qt.QString("Arc"))

    local luaMenu = menuBar:addMenuStr(qt.QString("Lua"))
    local luaScriptAction = luaMenu:addActionStr(qt.QString("Run script"))

    luaInterface:luaConnect(mainWindow:findChild("actionNew"), "triggered(bool)", new_file)
    luaInterface:luaConnect(mainWindow:findChild("actionOpen"), "triggered(bool)", open_file)
    luaInterface:luaConnect(mainWindow:findChild("actionSave_2"), "triggered(bool)", save_file)
    luaInterface:luaConnect(mainWindow:findChild("actionSave_As"), "triggered(bool)", save_file)

    luaInterface:luaConnect(mainWindow:findChild("actionUndo"), "triggered(bool)", function ()
        widget:undoManager():undo()
    end)
    luaInterface:luaConnect(mainWindow:findChild("actionRedo"), "triggered(bool)", function ()
        widget:undoManager():redo()
    end)

    luaInterface:luaConnect(luaScriptAction, "triggered(bool)", open_lua_script)
end

--Create a new window
function create_new_window(widget)
    local mainWindow = qt.loadUi(ui_path .. "/mainwindow.ui")
    mainWindow:setWindowTitle(qt.QObject.tr("LibreCAD"));
    mainWindow:setUnifiedTitleAndToolBarOnMac(true);
    mainWindow:setCentralWidget(widget)

    create_menu(mainWindow, widget)

    local layers = lc.Layers()
    layers:setMdiChild(widget)
    mainWindow:addDockWidget(2, layers)

    local linePatternSelect = lc.LinePatternSelect(mainWindow, true, true)
    linePatternSelect:setMdiChild(widget)

    local lineWidthSelect = lc.LineWidthSelect(mainWindow, true, true)
    local colorSelect = lc.ColorSelect(mainWindow, true, true)

    luaInterface:connect(layers, "layerChanged(lc::Layer_CSPtr)", linePatternSelect, "onLayerChanged(lc::Layer_CSPtr)")
    luaInterface:connect(layers, "layerChanged(lc::Layer_CSPtr)", lineWidthSelect, "onLayerChanged(lc::Layer_CSPtr)")
    luaInterface:connect(layers, "layerChanged(lc::Layer_CSPtr)", colorSelect, "onLayerChanged(lc::Layer_CSPtr)")

    local id = nextTableId(windows)
    local commandLine = add_commandline(mainWindow, id)
    add_toolbar(mainWindow, id, linePatternSelect, lineWidthSelect, colorSelect)
    addCadMdiChild(widget, id, commandLine)


    if(hideUI ~= true) then
        mainWindow:showMaximized()
    end
end

--Get next available index in the table
--It's used to store the CadMdiChild ids
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