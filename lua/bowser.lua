Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")
os = require("os")
Emits = require("lua/bowser/emit_fns")

-- TRACK_n constants
for i = 1,8 do
    _G['TRACK_'..i] = i
end
TRACKS = 8

-- Important timekeeping variables
TrackTimes = {0, 0, 0, 0, 0, 0, 0, 0}
TrackAdvances = {0, 0, 0, 0, 0, 0, 0, 0}
TrackEndTime = 0

-- Specific timing constants
SECOND = 1000
BOWSER_TIME_START_MAIN = 18087

-- Environment variables
Env = {}
Env['VIS_BOWSER_NO_AUDIO'] = ''
Env['VIS_BOWSER_ISOLATE'] = ''
Env['VIS_BOWSER_SKIP_INTRO'] = ''
Env['VIS_BOWSER_SKIP_TO'] = ''
Env['VIS_BOWSER_FRAME_TWEAK'] = ''
Env['VIS_BOWSER_MS_TWEAK'] = ''
Env['VIS_BOWSER_EXIT_MS'] = ''
Env['VIS_DEBUG'] = ''
Env['VIS_HELP'] = ''

for t = 1,TRACKS do
    Env['VIS_BOWSER_DUMP_TRACK'..t] = ''
end

if Vis.DEBUG > Vis.DEBUG_NONE then
    if Env['VIS_DEBUG'] == nil and Vis.DEBUG >= Vis.DEBUG_TRACE then
        Env['VIS_DEBUG'] = '1'
    end
    print(VisUtil.strobject(Env))
end

for k, _ in pairs(Env) do
    Env[k] = os.getenv(k)
end

-- Important timekeeping functions
function now(track)
    return TrackTimes[track]
end

function adv(track, length)
    return set(track, TrackTimes[track] + length)
end

function set(track, offset)
    TrackAdvances[track] = TrackAdvances[track] + 1
    TrackTimes[track] = offset
    TrackEndTime = math.max(TrackTimes[track], TrackEndTime)
    return now(track)
end

-- Select which file to play and load it
if Env['VIS_BOWSER_NO_AUDIO'] == nil then
    isolate = Env['VIS_BOWSER_ISOLATE']
    if isolate ~= nil then
        Vis.audio(Vis.flist, 0, "media/bowser-track-"..isolate..".mp3")
    else
        Vis.audio(Vis.flist, 0, "media/bowser-full.mp3")
    end
end

-- Position helpers for placing emits at precise locations
W = function(num, denom) return Vis.WIDTH * num / denom end
W_1_2 = W(1,2) -- 400
W_1_3 = W(1,3) -- 266.6...
W_2_3 = W(2,3) -- 533.3...
W_1_4 = W(1,4) -- 200
W_3_4 = W(3,4) -- 600
W_1_6 = W(1,6) -- 133.3...
W_5_6 = W(5,6) -- 666.6...
W_1_8 = W(1,8) -- 100

H = function(num, denom) return Vis.HEIGHT * num / denom end
H_1_2 = H(1,2) -- 300
H_1_3 = H(1,3) -- 200
H_2_3 = H(2,3) -- 400
H_1_4 = H(1,4) -- 150
H_3_4 = H(3,4) -- 450
H_1_6 = H(1,6) -- 100
H_5_6 = H(5,6) -- 500
H_1_8 = H(1,8) -- 75

-- Helper functions

-- Transparently wrap a function and provide debugging messages
function debug_wrap(fn, fnname)
    function wrapper(...)
        if fnname == nil then ffname = tostring(fn) end
        s = "%s(%s) -> %s"
        value = fn(...)
        print(s:format(fnname, VisUtil.strobject(...), value))
        if tonumber(Env['VIS_DEBUG']) > 1 then
            print(debug.traceback())
        end
        return value
    end
    return wrapper
end

-- Load a specific track's configuration, unless instructed not to
local function dotrack(track)
    isolate = Env['VIS_BOWSER_ISOLATE']
    if isolate == nil or isolate == tostring(track) then
        dofile("lua/bowser/track"..track..".lua")
    end
end

-- Helper function to generate a bound NextSchedule function for each track
local function GenNextScheduleFn(track, track_num)
    function NextSchedule()
        track.ScheduleIndex = track.ScheduleIndex + 1
        if track.ScheduleIndex > #track.SCHEDULE then
            print("ScheduleIndex " .. track.ScheduleIndex ..
                  " is greater than " ..  #track.SCHEDULE)
            print(debug.traceback())
            return track.SCHEDULE[#track.SCHEDULE]
        end
        return track.SCHEDULE[track.ScheduleIndex]
    end
    return NextSchedule
end

-- Adjust the audio starting position
-- Must be *before* scheduling to avoid conflicts
if Env['VIS_BOWSER_SKIP_TO'] ~= nil then
    VisUtil.seek_to(tonumber(Env['VIS_BOWSER_SKIP_TO']))
elseif Env['VIS_BOWSER_SKIP_INTRO'] ~= nil then
    VisUtil.seek_to(BOWSER_TIME_START_MAIN)
end

-- Frame offset adjustments; figure out where to start the frame list
-- Must be *before* scheduling to avoid conflicts
if Env['VIS_BOWSER_MS_TWEAK'] ~= nil then
    Vis.seekms(Vis.flist, 0, tonumber(Env['VIS_BOWSER_MS_TWEAK']))
elseif Env['VIS_BOWSER_FRAME_TWEAK'] ~= nil then
    Vis.seekframe(Vis.flist, 0, tonumber(Env['VIS_BOWSER_FRAME_TWEAK']))
end

-- Provide a way for tracks to share schedules:
-- T1, T2, ..., T8 are the tracks local namespaces. Their schedules are
-- T1.SCHEDULE, T2.SCHEDULE, ..., T8.SCHEDULE.
-- Only tracks 1, 2, 3, 4, and 8 have anything in them during the intro
for i = 1,TRACKS do
    local Tn = {} -- Track namespace
    _G['T'..i] = Tn
    Tn.ScheduleIndex = 0
    Tn.NextSchedule = GenNextScheduleFn(Tn, i)
    Tn.now = (function(n) return function() return now(i) end end)(i)
    Tn.adv = (function(n) return function(o) return adv(n, o) end end)(i)
    Tn.set = (function(n) return function(o) return set(n, o) end end)(i)
    Tn.next = (function(tn, n)
        return function() return set(n, tn.NextSchedule()) end
    end)(Tn, i)
    Tn.peek = (function(tn, n)
        return function() return tn.SCHEDULE[tn.ScheduleIndex+1] end
    end)(Tn, i)
    if Env['VIS_DEBUG'] ~= nil then
        Tn.now = debug_wrap(Tn.now, 'now')
        Tn.adv = debug_wrap(Tn.adv, 'adv')
        Tn.set = debug_wrap(Tn.set, 'set')
        Tn.next = debug_wrap(Tn.next, 'next')
    end
end

-- Actually load and calculate track schedules, all of them
for i = 1,TRACKS do
    dotrack(i)
    if Env['VIS_BOWSER_DUMP_TRACK'..i] ~= nil then
        print('T'..i..'.SCHEDULE = {' .. Tn.ScheduleIndex)
        for i = 1,#Tn.SCHEDULE,2 do
            prefix = "\t";
            if Tn.ScheduleIndex == i or Tn.ScheduleIndex == i+1 then
                prefix = " >> \t"
            end
            print(prefix..Tn.SCHEDULE[i]..", "..Tn.SCHEDULE[i+1]..",")
        end
        print("} -- #T"..i..".SCHEDULE = "..#Tn.SCHEDULE)
    end
end

-- Display help if configured to do so
if Env['VIS_HELP'] ~= nil then
    help_msg = {
        "Environment variables for bowser.lua:",
        "VIS_BOWSER_NO_AUDIO            Disable loading of bowser-full.mp3",
        "VIS_BOWSER_ISOLATE=<track>     Play only <track>, 1 to 8",
        "VIS_BOWSER_SKIP_INTRO          Seek to offset 18087; skip intro",
        "VIS_BOWSER_SKIP_TO=<msec>      Seek all to offset <msec> on load",
        "VIS_BOWSER_FRAME_TWEAK=<frame> Seek the flist to <frame> on load",
        "VIS_BOWSER_MS_TWEAK=<msec>     Seek the flist to <msec> on load",
        "VIS_BOWSER_EXIT_MS=<msec>      Force exit after <msec>",
        "VIS_BOWSER_DUMP_TRACKn         Print schedule of track n",
        "VIS_DEBUG                      Enable certain debugging messages",
        "VIS_HELP                       This message",
        "",
        "Useful offsets:",
        "\t 1223:   Track 1, intro, part 1",
        "\t 7108:   Track 1, intro, part 2",
        "\t13422:   Track 1, intro, part 3",
        "\t13675:   Track 3, intro, part 4, start of bottom fire",
        "\t18087:   Start of main song",
        "\t19211:   Start of track 4's repeating sections"
    }
    print(table.concat(help_msg, "\n"))
end

-- Figure out when we exit
exit_ms = tonumber(Env['VIS_BOWSER_EXIT_MS']) or TrackEndTime
Vis.exit(Vis.flist, exit_ms)

