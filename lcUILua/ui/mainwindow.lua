--Create empty new window
function new_file()
    register_all_commands()
    create_new_window()
    create_menu()
    connect_buttons()
    add_toolbar()
end

--Create dialog to enter Lua script
local function open_lua_script(widget, commandLine)
    local luaScript = lc.LuaScript(widget, commandLine)
    luaScript:show()
end

--Create main window menu
local function create_menu()
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

    local widget = mainWindow:getCadMdiChild()

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
local function connect_buttons()
	-- line
	mainWindow:connectMenuItem("action2_Point_Line", function() run_basic_operation(LineOperations, "_init_2p") end)
    mainWindow:connectMenuItem("actionPoint_Angle_Length_Line", function() run_basic_operation(LineOperations, "_init_pal") end)
    mainWindow:connectMenuItem("actionHorizontalLine", function() run_basic_operation(LineOperations, "_init_horizontal") end)
    mainWindow:connectMenuItem("actionVerticalLine", function() run_basic_operation(LineOperations, "_init_vertical") end)
    mainWindow:connectMenuItem("actionParallelLine", function() run_basic_operation(LineOperations, "_init_parallel") end)
    mainWindow:connectMenuItem("actionOrthogonalLine", function() run_basic_operation(LineOperations, "_init_orthogonal") end)
    mainWindow:connectMenuItem("actionRectangle", function() run_basic_operation(LineOperations, "_init_rectangle") end)
    mainWindow:connectMenuItem("actionPolygonCenCor", function() run_basic_operation(LineOperations, "_init_polygon_cencor") end)
    mainWindow:connectMenuItem("actionPolygonCenTan", function() run_basic_operation(LineOperations, "_init_polygon_centan") end)
    mainWindow:connectMenuItem("actionPolygonCorCor", function() run_basic_operation(LineOperations, "_init_polygon_corcor") end)

	-- circle
	mainWindow:connectMenuItem("actionCenter_Radius", function() run_basic_operation(CircleOperations, "_init_cr") end)
	mainWindow:connectMenuItem("actionCenter_Diameter", function() run_basic_operation(CircleOperations, "_init_cd") end)
	mainWindow:connectMenuItem("action2_Point_Circle", function() run_basic_operation(CircleOperations, "_init_2p") end)
	mainWindow:connectMenuItem("action3_Point_Circle_2", function() run_basic_operation(CircleOperations, "_init_3p") end)
	mainWindow:connectMenuItem("actionTan_Tan_Radius", function() run_basic_operation(CircleOperations, "_init_2t") end)
	mainWindow:connectMenuItem("actionTan_Tan_Tan", function() run_basic_operation(CircleOperations, "_init_3t") end)

	-- arc
	mainWindow:connectMenuItem("action3_Point_Arc", function() run_basic_operation(ArcOperations, "_init_3p") end)
	mainWindow:connectMenuItem("actionCenter_Start_End_2", function() run_basic_operation(ArcOperations, "_init_cse") end)

	-- ellipse
	mainWindow:connectMenuItem("actionEllipse_Axis", function() run_basic_operation(EllipseOperations) end)
    mainWindow:connectMenuItem("actionEllipse_Arc", function() run_basic_operation(EllipseOperations, "_init_arc") end)
    mainWindow:connectMenuItem("actionEllipse_FociPoints", function() run_basic_operation(EllipseOperations, "_init_foci") end)

	-- spline
	mainWindow:connectMenuItem("actionSpline", function() run_basic_operation(SplineOperations) end)

	-- polylines
	mainWindow:connectMenuItem("actionPolyline", function() create_lw_polyline() end)

	--dimensions
	mainWindow:connectMenuItem("actionLinear", function() run_basic_operation(DimLinearOperations) end)
	mainWindow:connectMenuItem("actionAligned", function() run_basic_operation(DimAlignedOperations) end)
	mainWindow:connectMenuItem("actionRadius", function() run_basic_operation(DimRadialOperations) end)
	mainWindow:connectMenuItem("actionDiameter", function() run_basic_operation(DimDiametricOperations) end)
	mainWindow:connectMenuItem("actionANG3PT", function() run_basic_operation(DimAngularOperations) end)
end
