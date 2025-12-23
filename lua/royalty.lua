
Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")
os = require("os")
Debug = VisUtil.Debug

-- Argument Parsing
function EstablishConfig()
    local function AddOption(options, option)
        options[#options+1] = {
            env = option[1],
            opt = option[2],
            typ = option[3],
            str = option[4]
        }
    end

    Config = {}
    AddOption(Config, {
        "VIS_NO_AUDIO",
        {"-q", "--quiet"},
        "nil",
        "do not play music"
    })
    AddOption(Config, {
        "VIS_SYNC_FRAMES",
        {"-s", "--sync"},
        "number",
        "number of frames to allow for audio sync (default: 12)"
    })
    AddOption(Config, {
        {"VIS_DEBUG", "LUA_DEBUG", "DEBUG"},
        {"-v", "--debug"},
        "nil",
        "enable debugging"
    })
    local parser = VisUtil.Args:new()
    for _, argspec in pairs(Config) do
        local opts = argspec.opt
        if type(opts) == "string" then opts = {opts} end
        local envs = argspec.env
        if type(envs) == "string" then envs = {envs} end
        for _, opt in ipairs(opts) do
            parser:add(opt, argspec.typ, argspec.str)
            for _, env in ipairs(envs) do
                parser:add_env(env, argspec.typ, argspec.str)
                parser:link_arg_env(opt, env)
            end
        end
    end
    local args, envs, err = parser:parse()
    if err ~= nil then
        error(err)
    end
    return args, envs
end

Arg, Env = EstablishConfig()
if not Env["VIS_NO_AUDIO"] then
    Vis.audio(Vis.flist, 0, "media/royalty.mp3")
    Vis.audiosync(Vis.flist, 0, Env["VIS_SYNC_FRAMES"] or 12) -- 0.2 seconds
    Vis.volume(0.5)
else
    VisUtil.Debug("VIS_NO_AUDIO set; disabling audio playback entirely")
end

function do_get_metrics()
    local fps = Vis.fps(Vis.script)
    print(("FPS: %f of %d"):format(fps, Vis.FPS_LIMIT))
end

Vis.on_keydown(function(...)
    VisUtil.Debug("Keydown:", ...)
end)

function main()
    local e = Emit:new({tag=1})
    e:count(100)
    e:life(500, 0)
    e:radius(2, 0)
    e:ds(2, 0.5)
    e:theta(math.pi, math.pi)
    e:color(0, 1, 1, 0, 0, 0)
    for i = 100, 10000, 300 do
        e:emit_at(i)
    end

    Vis.callback(Vis.flist, 200, Vis.script, "do_get_metrics()")
end

main()

-- vim: set ts=4 sts=4 sw=4:
