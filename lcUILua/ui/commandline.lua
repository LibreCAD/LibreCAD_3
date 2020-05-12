local commands = {}
local cliCommands = {}

--Write a message to the command line log
function message(message, id)
    if cliCommands[id] ~= nil then
        cliCommands[id]:write(tostring(message))
    end
end

--Register a new command
function add_command(name, callback)
    for i, cliCommand in pairs(cliCommands) do
        if(not cliCommand:addCommand(name, callback)) then
            print("Command " .. name .. " is already defined")
            break
        end
    end
end

--Coomand line command On/OFF
function cli_command_active(id, status)
    if cliCommands[id] ~= nil then
        cliCommands[id]:commandActive(status)
    end
end

--Configure command line to return raw text
function cli_get_text(id, getText)
    if cliCommands[id] ~= nil then
        cliCommands[id]:returnText(getText)
    end
end

--Create the command line and add it to the main window
function add_commandline(cliCommand, id)
    cliCommands[id] = cliCommand
    register_all_commands()

    return cliCommand
end

function register_all_commands()
    --Register every commands
    add_command("LINE", function(id) run_basic_operation(id, LineOperations) end)
    add_command("CIRCLE", function(id) run_basic_operation(id, CircleOperations) end)
    add_command("ARC", function(id) run_basic_operation(id, ArcOperations) end)
    add_command("ELLIPSE", function(id) run_basic_operation(id, EllipseOperations) end)
    add_command("ARCELLIPSE", function(id) run_basic_operation(id, EllipseOperations, "_init_arc") end)
    add_command("DIMALIGNED", function(id) run_basic_operation(id, DimAlignedOperations) end)
    add_command("DIMDIAMETRIC", function(id) run_basic_operation(id, DimDiametricOperations) end)
    add_command("DIMLINEAR", function(id) run_basic_operation(id, DimLinearOperations) end)
    add_command("DIMRADIAL", function(id) run_basic_operation(id, DimRadialOperations) end)
    add_command("SPLINE", function(id) run_basic_operation(id, SplineOperations) end)
    add_command("POLYLINE", function(id) create_lw_polyline(id) end)

    add_command("MOVE", function(id) run_basic_operation(id, MoveOperation) end)
    add_command("ROTATE", function(id) run_basic_operation(id, RotateOperation) end)
    add_command("COPY", function(id) run_basic_operation(id, CopyOperation) end)
    add_command("SCALE", function(id) run_basic_operation(id, ScaleOperation) end)
    add_command("REMOVE", function(id) run_basic_operation(id, RemoveOperation) end)
    add_command("TRIM", function(id) run_basic_operation(id, TrimOperation) end)
end
