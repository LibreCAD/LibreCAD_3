local commands = {}
local lastPoint = Coordinate(0,0)

function message(message)
    cliCommand:write(tostring(message))
end

function add_command(name, callback)
    if(cliCommand:addCommand(name)) then
        commands[name] = callback
    else
        print("Command " .. name .. " is already defined")
    end
end

function cli_get_text(getText)
    cliCommand:returnText(getText)
end

local function command(command)
    commands[command:toStdString()]()
end

local function coordinate(coordinate)
    event.trigger("point", coordinate)
end

local function relativeCoordinate(relative)
    local absolute = lastPoint:add(relative)
    message("-> " .. "x=" .. absolute:x() .. " y=" .. absolute:y() .. " z=" .. absolute:z())
    event.trigger("point", absolute)
end

local function number(number)
    event.trigger("number", number)
end

local function setLastPoint(point)
    lastPoint = point
end

local function text(text)
    event.trigger("text", text:toStdString())
end

function add_commandline()
    cliCommand = lc.CliCommand(mainWindow)
    mainWindow:addDockWidget(8, cliCommand)

    luaInterface:luaConnect(cliCommand, "commandEntered(QString)", command)
    luaInterface:luaConnect(cliCommand, "coordinateEntered(lc::geo::Coordinate)", coordinate)
    luaInterface:luaConnect(cliCommand, "relativeCoordinateEntered(lc::geo::Coordinate)", relativeCoordinate)
    luaInterface:luaConnect(cliCommand, "numberEntered(double)", number)
    luaInterface:luaConnect(cliCommand, "textEntered(QString)", text)

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

    event.register("point", setLastPoint)
end