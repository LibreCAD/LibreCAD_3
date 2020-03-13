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


function save_as_file(id)
    if(windows[id] == nil) then
        return
    end

    windows[id]:saveAsFile()
end


--Return the selected layer
function active_layer(id)
    if(windows[id] == nil) then
        return nil
    end

    return windows[id]:activeLayer()
end

--Return the selected viewport
function active_block(id)
    if(windows[id] == nil) then
        return nil
    end

    return windows[id]:activeViewport()
end


--Create MetaInfo containing every selected MetaTypes
function active_metaInfo(id)
    local widget = windows[id]

    if(widget == nil) then
        return nil
    end

    local metaInfo = widget:metaInfoManager():metaInfo()

    if(metaInfo == nil) then
        metaInfo = lc.meta.MetaInfo() -- nil != nullptr when calling a c++ function
    end


    return metaInfo
end

--Create dialog to enter Lua script
local function open_lua_script(widget, commandLine)
    local luaScript = lc.LuaScript(widget, commandLine)
    luaScript:show()
end

--Create main window menu
local function create_menu(mainWindow, widget, commandLine, id)
    local menuBar = mainWindow:menuBar()
    local drawMenu = menuBar:addMenuStr(qt.QString("Draw"))
    local lineAction = drawMenu:addActionStr(qt.QString("Line"))
    local circleAction = drawMenu:addActionStr(qt.QString("Circle"))
    local arcAction = drawMenu:addActionStr(qt.QString("Arc"))
	local ellipseAction = drawMenu:addActionStr(qt.QString("Ellipse"))
	local splineAction = drawMenu:addActionStr(qt.QString("Spline"))
	local polylineAction = drawMenu:addActionStr(qt.QString("Polyline"))

    local luaMenu = menuBar:addMenuStr(qt.QString("Lua"))
    local luaScriptAction = luaMenu:addActionStr(qt.QString("Run script"))

    luaInterface:luaConnect(mainWindow:findChild("actionNew"), "triggered(bool)", new_file)
    luaInterface:luaConnect(mainWindow:findChild("actionOpen"), "triggered(bool)", open_file)
    luaInterface:luaConnect(mainWindow:findChild("actionSave_2"), "triggered(bool)", function()
        save_file(widget.id)
    end)

    luaInterface:connect(mainWindow:findChild("actionExit"), "triggered(bool)", mainWindow, "close()")

    luaInterface:luaConnect(mainWindow:findChild("actionSave_As"), "triggered(bool)", function()
        save_as_file(widget.id)
    end)

    luaInterface:luaConnect(mainWindow:findChild("actionUndo"), "triggered(bool)", function ()
        widget:undoManager():undo()
    end)
    luaInterface:luaConnect(mainWindow:findChild("actionRedo"), "triggered(bool)", function ()
        widget:undoManager():redo()
    end)

    luaInterface:luaConnect(luaScriptAction, "triggered(bool)", function()
        open_lua_script(widget, commandLine)
    end)

    -- select options
    luaInterface:luaConnect(mainWindow:findChild("actionSelect_All"), "triggered(bool)", function()
        widget:viewer():docCanvas():selectAll()
    end)

    luaInterface:luaConnect(mainWindow:findChild("actionSelect_None"), "triggered(bool)", function()
        widget:viewer():docCanvas():removeSelection()
    end)

    luaInterface:luaConnect(mainWindow:findChild("actionInvert_Selection"), "triggered(bool)", function()
        widget:viewer():docCanvas():inverseSelection()
    end)

	-- connect draw menu options
	luaInterface:luaConnect(lineAction, "triggered(bool)", function() run_basic_operation(id, LineOperations) end)
	luaInterface:luaConnect(circleAction, "triggered(bool)", function() run_basic_operation(id, CircleOperations) end)
	luaInterface:luaConnect(arcAction, "triggered(bool)", function() run_basic_operation(id, ArcOperations) end)
	luaInterface:luaConnect(ellipseAction, "triggered(bool)", function() run_basic_operation(id, EllipseOperations) end)
	luaInterface:luaConnect(splineAction, "triggered(bool)", function() run_basic_operation(id, SplineOperations) end)
	luaInterface:luaConnect(polylineAction, "triggered(bool)", function() create_lw_polyline(id) end)
end

-- Connect menu buttons in the menu bar
local function connect_buttons(mainWindow, id)
	-- line
	luaInterface:luaConnect(mainWindow:findChild("action2_Point_Line"), "triggered(bool)", function() run_basic_operation(id, LineOperations) end)
	
	-- circle
	luaInterface:luaConnect(mainWindow:findChild("actionCenter_Radius"), "triggered(bool)", function() op = run_basic_operation(id, CircleOperations); op:_init_cr()  end)
	luaInterface:luaConnect(mainWindow:findChild("actionCenter_Diameter"), "triggered(bool)", function() op = run_basic_operation(id, CircleOperations); op:_init_cd() end)
	luaInterface:luaConnect(mainWindow:findChild("action2_Point_Circle"), "triggered(bool)", function() op = run_basic_operation(id, CircleOperations); op:_init_2p() end)
	luaInterface:luaConnect(mainWindow:findChild("action3_Point_Circle_2"), "triggered(bool)", function() op = run_basic_operation(id, CircleOperations); op:_init_3p() end)
	luaInterface:luaConnect(mainWindow:findChild("actionTan_Tan_Radius"), "triggered(bool)", function() op = run_basic_operation(id, CircleOperations); op:_init_2t() end)
	luaInterface:luaConnect(mainWindow:findChild("actionTan_Tan_Tan"), "triggered(bool)", function() op = run_basic_operation(id, CircleOperations); op:_init_3t() end)

	-- arc
	luaInterface:luaConnect(mainWindow:findChild("action3_Point_Arc"), "triggered(bool)", function() op = run_basic_operation(id, ArcOperations); op:_init_3p() end)
	luaInterface:luaConnect(mainWindow:findChild("actionCenter_Start_End_2"), "triggered(bool)", function() op = run_basic_operation(id, ArcOperations); op:_init_cse() end)

	-- ellipse
	luaInterface:luaConnect(mainWindow:findChild("actionEllipse"), "triggered(bool)", function() run_basic_operation(id, EllipseOperations) end)

	-- spline
	luaInterface:luaConnect(mainWindow:findChild("actionSpline"), "triggered(bool)", function() run_basic_operation(id, SplineOperations) end)

	-- polylines
	luaInterface:luaConnect(mainWindow:findChild("actionPolyline"), "triggered(bool)", function() create_lw_polyline(id) end)

	--dimensions
	luaInterface:luaConnect(mainWindow:findChild("actionLinear"), "triggered(bool)", function() run_basic_operation(id, DimLinearOperations) end)
	luaInterface:luaConnect(mainWindow:findChild("actionAligned"), "triggered(bool)", function() run_basic_operation(id, DimAlignedOperations) end)
	luaInterface:luaConnect(mainWindow:findChild("actionRadius"), "triggered(bool)", function() run_basic_operation(id, DimRadialOperations) end)
	luaInterface:luaConnect(mainWindow:findChild("actionDiameter"), "triggered(bool)", function() run_basic_operation(id, DimDiametricOperations) end)
	luaInterface:luaConnect(mainWindow:findChild("actionANG3PT"), "triggered(bool)", function() run_basic_operation(id, DimAngularOperations) end)
end

--Create a new window
function create_new_window(widget)
    local mainWindow = qt.loadUi(ui_path .. "/mainwindow.ui")
    mainWindow:setWindowTitle(qt.QObject.tr("LibreCAD"));
    mainWindow:setUnifiedTitleAndToolBarOnMac(true);
    mainWindow:setCentralWidget(widget)

    local layers = lc.Layers()
    layers:setMdiChild(widget)
    mainWindow:addDockWidget(2, layers)

    local linePatternSelect = lc.LinePatternSelect(widget:metaInfoManager(), mainWindow, true, true)
    linePatternSelect:setMdiChild(widget)

    local lineWidthSelect = lc.LineWidthSelect(widget:metaInfoManager(), mainWindow, true, true)
    local colorSelect = lc.ColorSelect(widget:metaInfoManager(), mainWindow, true, true)

    luaInterface:connect(layers, "layerChanged(lc::meta::Layer_CSPtr)", linePatternSelect, "onLayerChanged(lc::meta::Layer_CSPtr)")
    luaInterface:connect(layers, "layerChanged(lc::meta::Layer_CSPtr)", lineWidthSelect, "onLayerChanged(lc::meta::Layer_CSPtr)")
    luaInterface:connect(layers, "layerChanged(lc::meta::Layer_CSPtr)", colorSelect, "onLayerChanged(lc::meta::Layer_CSPtr)")

    local id = nextTableId(windows)
    local commandLine = add_commandline(mainWindow, id)
    add_toolbar(mainWindow, id, linePatternSelect, lineWidthSelect, colorSelect)
    addCadMdiChild(widget, id, commandLine)

    create_menu(mainWindow, widget, commandLine, id)
	connect_buttons(mainWindow, id)

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
