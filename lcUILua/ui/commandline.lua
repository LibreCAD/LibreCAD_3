local commands = {}

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

local function command(command)
    commands[command:toStdString()]()
end

local function coordinate(coordinate)
    event.trigger("point", coordinate)
end

local function number(number)
    event.trigger("number", number)
end

function add_commandline()
    cliCommand = lc.CliCommand(mainWindow)
    mainWindow:addDockWidget(8, cliCommand)

    luaInterface:luaConnect(cliCommand, "commandEntered(QString)", command)
    luaInterface:luaConnect(cliCommand, "coordinateEntered(lc::geo::Coordinate)", coordinate)
    luaInterface:luaConnect(cliCommand, "numberEntered(double)", number)

    add_command("LINE", create_line)
end