
Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")
os = require("os")
Letters = require("letters")
Debug = VisUtil.Debug

-- Argument Parsing
function EstablishConfig()
    local function AddOption(options, option)
        options[#options+1] = {
            env = option[1],
            opt = option[2],
            typ = option[3],
            str = option[4],
            def = option[5]
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
        "number of frames to allow for audio sync",
        12
    })
    AddOption(Config, {
        {"VIS_DEBUG", "LUA_DEBUG", "DEBUG"},
        {"-v", "--debug"},
        "nil",
        "enable debugging"
    })
    AddOption(Config, {
        "VIS_VOLUME",
        {"-V", "--volume"},
        "number",
        "volume percentage",
        50
    })
    local parser = VisUtil.Args:new()
    for _, argspec in pairs(Config) do
        local opts = argspec.opt
        if type(opts) == "string" then opts = {opts} end
        local envs = argspec.env
        if type(envs) == "string" then envs = {envs} end
        for _, opt in ipairs(opts) do
            parser:add(opt, argspec.typ, argspec.str)
            if argspec.def then
                parser:set_default(opt, argspec.def)
            end
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

INTRO_DELAY = 1000

Arg, Env = EstablishConfig()
if Env["DEBUG"] then
    print(require("inspect").inspect(Arg))
end

if not Env["VIS_NO_AUDIO"] then
    Vis.audio(Vis.flist, INTRO_DELAY, "media/royalty.mp3")
    Vis.callback(Vis.flist, INTRO_DELAY, Vis.script,
        ("Vis.volume(%f)"):format((Env["VIS_VOLUME"] or 50) / 100))
else
    VisUtil.Debug("VIS_NO_AUDIO set; disabling audio playback entirely")
end

function do_get_metrics()
    local fps = Vis.fps(Vis.script)
    print(("FPS: %f of %d"):format(fps, Vis.FPS_LIMIT))
end

Vis.on_keydown(function(key)
    VisUtil.Debug("Keydown: " .. key)
    if key == "Left" then
        Vis.gotoframe(Vis.flist, INTRO_DELAY)
        Vis.seek(0)
        VisUtil.Debug("Received LEFT input; reverting to beginning of song")
    end
end)

W, H = Vis.WIDTH, Vis.HEIGHT

function emit_intro_message()
    local zoom = 4  -- letter size
    local e = Emit:new({tag="intro"})
    e:count(zoom*2)
    e:radius(1)
    e:ds(0)
    e:theta(0, math.pi)
    e:life(INTRO_DELAY)
    e:color(0, 0.8, 0.4, 0.2, .1, 0)
    e:blender(Vis.BLEND_EASING)

    local function emit_char(frame, c, x, y, lx, ly, zoom)
        Letters.map_fn_xy(c:upper(), function(bx, by)
            e:center(x + zoom*(bx+lx), y + zoom*(by+ly), zoom/2, zoom/2)
            e:emit(frame)
        end)
    end

    local function emit_message(frame, msg, x, y, zoom)
        for idx, ord in ipairs(table.pack(msg:byte(1, #msg))) do
            chr = string.char(ord)
            emit_char(frame, chr, x, y, (idx-1)*(Letters.LETTER_WIDTH+1), 0, zoom)
        end
    end

    local function emit_center_message(frame, msg, x, y, zoom)
        local width, length = Letters.find_extents(msg)
        emit_message(frame, msg, x - width*zoom/2, y - length*zoom/2, zoom)
    end

    local function emit_lines(frame, lines, x, y, zoom)
        local line_height = 1.5 * Letters.LETTER_HEIGHT * zoom
        for idx, line in ipairs(lines) do
            emit_center_message(frame, line, x, y + (idx-1)*line_height, zoom)
        end
    end

    emit_lines(1, {
        "Royalty",
        "Ezgod, Maestro Chives, Neoni",
        "NCS - No Copyright Sounds"
    }, W/2, H/2, zoom)
end

function main()
    local dead_time = 0
    local now = INTRO_DELAY + dead_time
    local emit = Emit:new({tag=1})
    emit:center(W/2, H/2, 0, 0)
    emit:set("s", 100)
    emit:set("us", 100)
    emit:count(10000)
    emit:life(600, 0)
    emit:radius(4, 0)
    emit:ds(0, 0.1)
    emit:theta(math.pi, math.pi)
    emit:color(0, 1, 1, 0, 0, 0)
    emit:emit_at(now)
    emit:blender(Vis.NO_BLEND)

    now = now + 500
    emit:set("tag", 2)
    emit:color(1, 1, 1, 0, 0, 0)
    emit:center(W/2, H/2, W/8, H/5)
    emit:ds(0.1, 0)
    emit:theta(math.pi/2, 0)
    emit:life(2000, 0)
    emit:emit_at(now)

    now = now + 300
    Vis.mutate(Vis.flist, now, Vis.MUTATE_SET_DY, 0.5, 0)

    now = now + 1000
    Vis.mutate(Vis.flist, now, Vis.MUTATE_SET_DY, -0.5, 0)
end

Vis.on_mousescroll(function(xoffset, yoffset)
    VisUtil.Debug(("Scroll: %d %d"):format(xoffset, yoffset))
end)

emit_intro_message()
main()
--[[
for i = 0, 100 do
Vis.emit(Vis.flist, 200, 1000 + 10*i, {
    x = W/2,
    y = H/2,
    ux = 0, uy = 0,
    s = 200,
    us = 100,
    ds = 1,
    uds = 0.5,
    theta = math.pi,
    utheta = math.pi,
    r = 1,
    g = 1,
    b = 1,
    ur = 0,
    ug = 0,
    ub = 0,
    rad = 1,
    life = 1000,
    ulife = 100,
})
end
]]

-- vim: set ts=4 sts=4 sw=4:
