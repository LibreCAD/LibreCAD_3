function createSandboxEnv()
    local env = {}
    for k, v in pairs(_G) do
        env[k] = v
    end

    env.loadstring = function(str)
        return load(str, str, "t", createSandboxEnv())
    end

    env.dofile = function(file)
        return loadfile(file, "t", createSandboxEnv())
    end

    env.load = function(...)
        print("Function load is disabled. Use loadstring instead.")
    end

    env.loadfile = function(...)
        print("Function loadfile is disabled. Use dofile instead.")
    end

    env.require = function(...)
        print("Function require is disabled.")
    end

    env.package = nil
    env.io = nil

    env.os = {
        clock = os.clock,
        difftime = os.difftime
    }

    return env
end

function load_plugins() -- TODO: remove
    local plugins = luaInterface:pluginList(plugin_path)
    for k, v in pairs(plugins) do
        plugin, err = loadfile(plugin_path .. "/" .. v .. "/plugin.lua", "t", createSandboxEnv())
        if(err == nil) then
            plugin()
        else
            message(err)
        end
    end
end