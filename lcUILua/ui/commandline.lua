local commands = {}
local lastPoint = Coordinate(0,0)

--Write a message to the command line log
function message(message)
    cliCommand:write(tostring(message))
end

--Register a new command
function add_command(name, callback)
    if(cliCommand:addCommand(name)) then
        commands[name] = callback
    else
        print("Command " .. name .. " is already defined")
    end
end

--Configure command line to return raw text
function cli_get_text(getText)
    cliCommand:returnText(getText)
end

--Execute a command from command line
function command(command)
    commands[command:toStdString()]()
end

--Send an event when coordinate is entered
function coordinate(coordinate)
    luaInterface:triggerEvent('point', coordinate)
end

--Send an event when relative coordinate is entered and show real coordinate in command line
function relativeCoordinate(relative)
    local absolute = lastPoint:add(relative)
    message("-> " .. "x=" .. absolute:x() .. " y=" .. absolute:y() .. " z=" .. absolute:z())
    luaInterface:triggerEvent('point', absolute)
end

--Send an event when a number is entered
function number(number)
    luaInterface:triggerEvent('number', number)
end

--Store the point in memory when needed for relative coordinates
local function setLastPoint(point)
    lastPoint = point
end

--Send an event when text is entered
function text(text)
    luaInterface:triggerEvent('text', text:toStdString())
end

--Create the command line and add it to the main window
function add_commandline()
    cliCommand = lc.CliCommand(mainWindow)
    mainWindow:addDockWidget(8, cliCommand)

    luaInterface:luaConnect(cliCommand, "commandEntered(QString)", command)
    luaInterface:luaConnect(cliCommand, "coordinateEntered(lc::geo::Coordinate)", coordinate)
    luaInterface:luaConnect(cliCommand, "relativeCoordinateEntered(lc::geo::Coordinate)", relativeCoordinate)
    luaInterface:luaConnect(cliCommand, "numberEntered(double)", number)
    luaInterface:luaConnect(cliCommand, "textEntered(QString)", text)

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
end