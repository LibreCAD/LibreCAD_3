op = {}

--Create empty new window
function new_file()
    --local cadMdiChild = lc.CadMdiChild()
    --cadMdiChild:newDocument()

    --create_new_window(cadMdiChild);
    local id = nextTableId(windows)
    windows[id] = mainWindow:getCadMdiChild()
    windows[id].id = id
    add_commandline(mainWindow:getCliCommand(), id)
    --addCadMdiChild(mainWindow:getCadMdiChild(), id, mainWindow:getCliCommand())

    create_new_window(id)
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
local function create_menu(id)
    --[[local menuBar = mainWindow:menuBar()
    local drawMenu = menuBar:addMenuStr(qt.QString("Draw"))
    local lineAction = drawMenu:addActionStr(qt.QString("Line"))
    local circleAction = drawMenu:addActionStr(qt.QString("Circle"))
    local arcAction = drawMenu:addActionStr(qt.QString("Arc"))
	local ellipseAction = drawMenu:addActionStr(qt.QString("Ellipse"))
	local splineAction = drawMenu:addActionStr(qt.QString("Spline"))
	local polylineAction = drawMenu:addActionStr(qt.QString("Polyline"))

    local luaMenu = menuBar:addMenuStr(qt.QString("Lua"))
    local luaScriptAction = luaMenu:addActionStr(qt.QString("Run script"))]]--

    mainWindow:connectMenuItem("actionNew", new_file)
    mainWindow:connectMenuItem("actionOpen", open_file)
    mainWindow:connectMenuItem("actionSave_2", function()
        save_file(widget.id)
    end)

    --luaInterface:connect(mainWindow:findChild("actionExit"), "triggered(bool)", mainWindow, "close()")

    mainWindow:connectMenuItem("actionSave_As", function()
        save_as_file(widget.id)
    end)

    mainWindow:connectMenuItem("actionUndo", function ()
        widget:undoManager():undo()
    end)
    mainWindow:connectMenuItem("actionRedo", function ()
        widget:undoManager():redo()
    end)

    --[[luaInterface:luaConnect(luaScriptAction, "triggered(bool)", function()
        open_lua_script(widget, commandLine)
    end)]]--

    -- select options
    mainWindow:connectMenuItem("actionSelect_All", function()
        widget:viewer():docCanvas():selectAll()
    end)

    mainWindow:connectMenuItem("actionSelect_None", function()
        widget:viewer():docCanvas():removeSelection()
    end)

    mainWindow:connectMenuItem("actionInvert_Selection", function()
        widget:viewer():docCanvas():inverseSelection()
    end)

	--[[-- connect draw menu options
	luaInterface:luaConnect(lineAction, "triggered(bool)", function() run_basic_operation(id, LineOperations) end)
	luaInterface:luaConnect(circleAction, "triggered(bool)", function() run_basic_operation(id, CircleOperations) end)
	luaInterface:luaConnect(arcAction, "triggered(bool)", function() run_basic_operation(id, ArcOperations) end)
	luaInterface:luaConnect(ellipseAction, "triggered(bool)", function() run_basic_operation(id, EllipseOperations) end)
	luaInterface:luaConnect(splineAction, "triggered(bool)", function() run_basic_operation(id, SplineOperations) end)
	luaInterface:luaConnect(polylineAction, "triggered(bool)", function() create_lw_polyline(id) end)]]--
end

-- Connect menu buttons in the menu bar
local function connect_buttons(id)
	-- line
	mainWindow:connectMenuItem("action2_Point_Line", function() run_basic_operation(id, LineOperations, "_init_2p") end)
    mainWindow:connectMenuItem("actionPoint_Angle_Length_Line", function() run_basic_operation(id, LineOperations, "_init_pal") end)
    mainWindow:connectMenuItem("actionHorizontalLine", function() run_basic_operation(id, LineOperations, "_init_horizontal") end)
    mainWindow:connectMenuItem("actionVerticalLine", function() run_basic_operation(id, LineOperations, "_init_vertical") end)
    mainWindow:connectMenuItem("actionParallelLine", function() run_basic_operation(id, LineOperations, "_init_parallel") end)
    mainWindow:connectMenuItem("actionOrthogonalLine", function() run_basic_operation(id, LineOperations, "_init_orthogonal") end)
    mainWindow:connectMenuItem("actionRectangle", function() run_basic_operation(id, LineOperations, "_init_rectangle") end)
    mainWindow:connectMenuItem("actionPolygonCenCor", function() run_basic_operation(id, LineOperations, "_init_polygon_cencor") end)
    mainWindow:connectMenuItem("actionPolygonCenTan", function() run_basic_operation(id, LineOperations, "_init_polygon_centan") end)
    mainWindow:connectMenuItem("actionPolygonCorCor", function() run_basic_operation(id, LineOperations, "_init_polygon_corcor") end)

	-- circle
	mainWindow:connectMenuItem("actionCenter_Radius", function() run_basic_operation(id, CircleOperations, "_init_cr") end)
	mainWindow:connectMenuItem("actionCenter_Diameter", function() run_basic_operation(id, CircleOperations, "_init_cd") end)
	mainWindow:connectMenuItem("action2_Point_Circle", function() run_basic_operation(id, CircleOperations, "_init_2p") end)
	mainWindow:connectMenuItem("action3_Point_Circle_2", function() run_basic_operation(id, CircleOperations, "_init_3p") end)
	mainWindow:connectMenuItem("actionTan_Tan_Radius", function() run_basic_operation(id, CircleOperations, "_init_2t") end)
	mainWindow:connectMenuItem("actionTan_Tan_Tan", function() run_basic_operation(id, CircleOperations, "_init_3t") end)

	-- arc
	mainWindow:connectMenuItem("action3_Point_Arc", function() run_basic_operation(id, ArcOperations, "_init_3p") end)
	mainWindow:connectMenuItem("actionCenter_Start_End_2", function() run_basic_operation(id, ArcOperations, "_init_cse") end)

	-- ellipse
	mainWindow:connectMenuItem("actionEllipse_Axis", function() run_basic_operation(id, EllipseOperations) end)
    mainWindow:connectMenuItem("actionEllipse_Arc", function() run_basic_operation(id, EllipseOperations, "_init_arc") end)
    mainWindow:connectMenuItem("actionEllipse_FociPoints", function() run_basic_operation(id, EllipseOperations, "_init_foci") end)

	-- spline
	mainWindow:connectMenuItem("actionSpline", function() run_basic_operation(id, SplineOperations) end)

	-- polylines
	mainWindow:connectMenuItem("actionPolyline", function() create_lw_polyline(id) end)

	--dimensions
	mainWindow:connectMenuItem("actionLinear", function() run_basic_operation(id, DimLinearOperations) end)
	mainWindow:connectMenuItem("actionAligned", function() run_basic_operation(id, DimAlignedOperations) end)
	mainWindow:connectMenuItem("actionRadius", function() run_basic_operation(id, DimRadialOperations) end)
	mainWindow:connectMenuItem("actionDiameter", function() run_basic_operation(id, DimDiametricOperations) end)
	mainWindow:connectMenuItem("actionANG3PT", function() run_basic_operation(id, DimAngularOperations) end)
end

--Create a new window
function create_new_window(id)
    create_menu(id)
    connect_buttons(id)
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
