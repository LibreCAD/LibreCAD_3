local commands = {}
local cliCommands = {}
local lastPoint = Coordinate(0,0)

--Write a message to the command line log
function message(message, id)
    if cliCommands[id] ~= nil then
        cliCommands[id]:write(tostring(message))
    end
end

--Register a new command
function add_command(name, callback)
    for i, cliCommand in pairs(cliCommands) do
        if(cliCommand:addCommand(name)) then
            commands[name] = callback
        else
            print("Command " .. name .. " is already defined")
            break
        end
    end
end

--Configure command line to return raw text
function cli_get_text(id, getText)
    if cliCommands[id] ~= nil then
        cliCommands[id]:returnText(getText)
    end
end

--Execute a command from command line
function command(command)
    commands[command:toStdString()]()
end

--Store the point in memory when needed for relative coordinates
local function setLastPoint(point)
    lastPoint = point
end

--Create the command line and add it to the main window
function add_commandline(mainWindow, id)
    local cliCommand = lc.CliCommand(mainWindow)
    mainWindow:addDockWidget(8, cliCommand)
    cliCommands[id] = cliCommand

    luaInterface:luaConnect(cliCommand, "commandEntered(QString)", command)

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
            text = text:toStdString(),
            widget = mainWindow:centralWidget()
        })
    end)

    --Register every commands
    add_command("LINE", create_line)
    add_command("CIRCLE", create_circle)
    add_command("ARC", create_arc)
    add_command("ELLIPSE", create_ellipse)
    add_command("ARCELLIPSE", create_arc_ellipse)
    add_command("DIMALIGNED", create_dim_aligned)
    add_command("DIMDIAMETRIC", create_dim_diametric)
    add_command("DIMLINEAR", create_dim_linear)
    add_command("DIMRADIAL", create_dim_radial)
    add_command("SPLINE", create_spline)

    add_command("MOVE", move_selected_entities)
    add_command("ROTATE", rotate_selected_entities)
    add_command("COPY", copy_selected_entities)
    add_command("SCALE", scale_selected_entities)
    add_command("REMOVE", remove_selected_entities)
    add_command("TRIM", trim_entity)

    luaInterface:registerEvent('point', setLastPoint)

    return cliCommand
end