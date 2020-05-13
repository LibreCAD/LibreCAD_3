--Write a message to the command line log
function message(message)
    local cliCommand = mainWindow:getCliCommand()
    if cliCommand ~= nil then
        cliCommand:write(tostring(message))
    end
end

--Register a new command
function add_command(name, callback)
    local cliCommand = mainWindow:getCliCommand()
    if(not cliCommand:addCommand(name, callback)) then
        print("Command " .. name .. " is already defined")
    end
end

function register_all_commands()
    --Register every commands
    add_command("LINE", function() run_basic_operation(LineOperations) end)
    add_command("CIRCLE", function() run_basic_operation(CircleOperations) end)
    add_command("ARC", function() run_basic_operation(ArcOperations) end)
    add_command("ELLIPSE", function() run_basic_operation(EllipseOperations) end)
    add_command("ARCELLIPSE", function() run_basic_operation(EllipseOperations, "_init_arc") end)
    add_command("DIMALIGNED", function() run_basic_operation(DimAlignedOperations) end)
    add_command("DIMDIAMETRIC", function() run_basic_operation(DimDiametricOperations) end)
    add_command("DIMLINEAR", function() run_basic_operation(DimLinearOperations) end)
    add_command("DIMRADIAL", function() run_basic_operation(DimRadialOperations) end)
    add_command("SPLINE", function() run_basic_operation(SplineOperations) end)
    add_command("POLYLINE", function() create_lw_polyline() end)

    add_command("MOVE", function() run_basic_operation(MoveOperation) end)
    add_command("ROTATE", function() run_basic_operation(RotateOperation) end)
    add_command("COPY", function() run_basic_operation(CopyOperation) end)
    add_command("SCALE", function() run_basic_operation(ScaleOperation) end)
    add_command("REMOVE", function() run_basic_operation(RemoveOperation) end)
    add_command("TRIM", function() run_basic_operation(TrimOperation) end)
end
