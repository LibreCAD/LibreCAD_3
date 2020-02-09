local commands = {}
local cliCommands = {}
local lastPoint = lc.geo.Coordinate(0,0)

--Write a message to the command line log
function message(message, id)
    if cliCommands[id] ~= nil then
        cliCommands[id]:write(tostring(message))
    end
end

--Register a new command
function add_command(name, callback)
    commands[name] = callback

    for i, cliCommand in pairs(cliCommands) do
        if(not cliCommand:addCommand(name)) then
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

-- focus cli command
function focusClicommand()
    cliCommand:setFocus()
end

--Configure command line to return raw text
function cli_get_text(id, getText)
    if cliCommands[id] ~= nil then
        cliCommands[id]:returnText(getText)
    end
end

--Execute a command from command line
function command(id, command)
    commands[qt.QString.toStdString(command)](id)
end

--Store the point in memory when needed for relative coordinates
local function setLastPoint(point)
    lastPoint = point
end

--Create the command line and add it to the main window
function add_commandline(mainWindow, id)
    cliCommand = lc.CliCommand(mainWindow)
    mainWindow:addDockWidget(8, cliCommand)
    cliCommands[id] = cliCommand

    luaInterface:luaConnect(cliCommand, "commandEntered(QString)", function(...) command(id, ...) end)

    luaInterface:luaConnect(cliCommand, "finishOperation()", function()
        luaInterface:triggerEvent('operationFinished', 0)
        luaInterface:triggerEvent('finishOperation', 0)
    end)

    luaInterface:luaConnect(cliCommand, "coordinateEntered(lc::geo::Coordinate)", function(coordinate)
        luaInterface:triggerEvent('point', {
            position = coordinate,
            widget = mainWindow:centralWidget()
        })
    end)

    luaInterface:luaConnect(cliCommand, "relativeCoordinateEntered(lc::geo::Coordinate)", function(relative)
        local absolute = lastPoint:add(relative)
        message("-> " .. "x=" .. absolute:x() .. " y=" .. absolute:y() .. " z=" .. absolute:z(), id)
        luaInterface:triggerEvent('point', {
            position = absolute,
            widget = mainWindow:centralWidget()
        })
    end)

    luaInterface:luaConnect(cliCommand, "numberEntered(double)", function(number)
        luaInterface:triggerEvent('number', {
            number = number,
            widget = mainWindow:centralWidget()
        })
    end)

    luaInterface:luaConnect(cliCommand, "textEntered(QString)", function(text)
        luaInterface:triggerEvent('text', {
            text = qt.QString.toStdString(text),
            widget = mainWindow:centralWidget()
        })
    end)

    for name, cb in pairs(commands) do
        cliCommand:addCommand(name)
    end

    luaInterface:registerEvent('point', setLastPoint)

    return cliCommand
end

--Register every commands
add_command("LINE", function(id) run_basic_operation(id, LineOperations) end)
add_command("CIRCLE", function(id) run_basic_operation(id, CircleOperations) end)
add_command("ARC", function(id) run_basic_operation(id, ArcOperations) end)
add_command("ELLIPSE", function(id) run_basic_operation(id, EllipseOperations) end)
add_command("ARCELLIPSE", function(id) run_basic_operation(id, EllipseOperations, true) end)
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
