Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")
os = require("os")

TRACK_1 = 1
TRACK_2 = 2
TRACK_3 = 3
TRACK_4 = 4
TRACK_5 = 5
TRACK_6 = 6
TRACK_7 = 7
TRACK_8 = 8
TRACKS = 8
TrackTimes = {0, 0, 0, 0, 0, 0, 0, 0}

SECOND = 1000

function now(track)
    return TrackTimes[track]
end

function adv(track, length)
    TrackTimes[track] = TrackTimes[track] + length
end

function settime(track, offset)
    TrackTimes[track] = offset
end

if os.getenv('VIS_NO_AUDIO') == nil then
    t = os.getenv('VIS_ISOLATE')
    if t ~= nil then
        Vis.audio("media/bowser-"..t..".wav")
    else
        Vis.audio("media/Bowser.wav")
    end
end

function asserteq(a, b)
    if a ~= b then
        print(a.." is not equal to "..b)
        assert(a == b)
    end
end

function emit_spiral(steps)
    local MAXLIFE = 10/3*SECOND
    local MAXLIFE_ADJ = 1/3*SECOND

    et = VisUtil.make_emit_table()
    center_emit_table(et)
    et.radius = 2
    et.ds = 2
    et.uds = 1.75
    et.life = MAXLIFE
    et.ulife = MAXLIFE_ADJ
    VisUtil.color_emit_table(et, 0, 100, 200, 0, 10, 50)
    for i = 0, steps do
        et.count = 1000
        et.when = i
        et.theta = 2*math.pi * i / steps
        et.utheta = 2*math.pi / steps
        VisUtil.emit_table(et)
    end
    return steps + MAXLIFE + MAXLIFE_ADJ
end

function emit_line_v(start, x, r, g, b, ur, ug, ub)
    local MAXLIFE = SECOND/3
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, x, 0, 0, Vis.HEIGHT)
    et.radius = 2
    et.ds = 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0)
    et.count = 200
    et.when = start
    et.theta = math.pi/2
    et.utheta = 0.1
    VisUtil.emit_table(et)
    et.theta = math.pi*3/2
    et.utheta = 0.1
    VisUtil.emit_table(et)
end

function emit_line_h(start, y, r, g, b, ur, ug, ub)
    local MAXLIFE = SECOND/3
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, 0, y, Vis.WIDTH, 0)
    et.radius = 2
    et.ds = 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0)
    et.count = 200
    et.when = start
    et.theta = 0
    VisUtil.emit_table(et)
    et.theta = math.pi
    VisUtil.emit_table(et)
end

W_1_2 = Vis.WIDTH / 2 -- 400
W_1_3 = Vis.WIDTH / 3 -- 266.6...
W_2_3 = W_1_3 * 2     -- 533.3...
W_1_4 = Vis.WIDTH / 4 -- 200
W_3_4 = W_1_4 * 3     -- 600
W_1_6 = Vis.WIDTH / 6 -- 133.3...
W_5_6 = W_1_6 * 5     -- 666.6...
W_1_8 = Vis.WIDTH / 8 -- 100

H_1_2 = Vis.HEIGHT / 2 -- 300
H_1_3 = Vis.HEIGHT / 3 -- 200
H_2_3 = H_1_3 * 2      -- 400
H_1_4 = Vis.HEIGHT / 4 -- 150
H_3_4 = H_1_4 * 3      -- 450
H_1_6 = Vis.HEIGHT / 6 -- 100
H_5_6 = H_1_6 * 5      -- 500
H_1_8 = Vis.HEIGHT / 8 -- 75

local function dotrack(track)
    isolate = os.getenv('VIS_ISOLATE')
    if isolate == nil or isolate == tostring(track) then
        dofile("lua/bowser/track"..track..".lua")
    end
end

-- Only tracks 1, 2, 3, 4, and 8 have anything in them during the intro
dotrack(1)
dotrack(2)
dotrack(3)
dotrack(4)
dotrack(5)
dotrack(6)
dotrack(7)
dotrack(8)

if os.getenv('VIS_HELP') ~= nil then
    help_msg = {
        "Environment variables for bowser.lua:",
        "VIS_NO_AUDIO                   Disable loading of Bowser.wav",
        "VIS_ISOLATE=<track>            Play only <track>, 1 to 8",
        "VIS_BOWSER_SKIP_TO=<msec>      Seek all to offset <msec> on load",
        "VIS_BOWSER_FRAME_TWEAK=<frame> Seek the flist to <frame> on load",
        "VIS_BOWSER_MS_TWEAK=<msec>     Seek the flist to <msec> on load",
        "VIS_BOWSER_EXIT_MS=<msec>      Force exit after <msec>",
        "VIS_HELP                       This message",
        "",
        "Useful offsets:",
        "\t 1223:   Track 1, intro, part 1",
        "\t 7108:   Track 1, intro, part 2",
        "\t13422:   Track 1, intro, part 3",
        "\t18087:   Start of main song",
        "\t13675:   Track 3, intro, part 4, start of bottom fire"
    }
    print(table.concat(help_msg, "\n"))
end

skip_to = os.getenv('VIS_BOWSER_SKIP_TO')
if skip_to ~= nil then
    VisUtil.seek_to(tonumber(skip_to))
end

frame_tweak = os.getenv('VIS_BOWSER_FRAME_TWEAK')
if frame_tweak == nil and os.getenv('VIS_BOWSER_MS_TWEAK') ~= nil then
    frame_tweak = Vis.msec2frames(os.getenv('VIS_BOWSER_MS_TWEAK'))
end
if frame_tweak ~= nil then
    Vis.seekframe(Vis.flist, 0, frame_tweak)
end

exit_ms = os.getenv('VIS_BOWSER_EXIT_MS') or TrackTimes[TRACK_1] + 500
Vis.exit(Vis.flist, exit_ms)

