
-- Imagine this as a Python docstring:
VIS_BOWSER_HELP_STRING = [[
The Final Product: Bowser Visualization

This script and all sub-scripts are to schedule and perform the
visualization I've imagined for this project since shortly after its
inception. It is separated into several distinct parts:

1) The driver: bowser.lua
2) The tracks: bowser/track{1..8}.lua
3) The track implementations: bowser/t{1..8}*.lua

This driver works in several distinct phases:

1) Define all global variables, constants, and environments.
    a) Timekeeping variables and constants
    b) Environment variables and debugging configuration
    c) Timekeeping functions (the lowest-level ones)
    d) Music playback functions
    e) Position constants
    f) Starting position configuration (music and visualization)
2) Define per-track variables, constants, and environments.
    a) Track namespace
    b) Track-specific timekeeping functions
3) Load and execute each track in sequence
    a) Specifically load and execute the track script
    b) Debugging (if configured)
4) Help information (via the VIS_HELP environment variable)
5) Exit time configuration

The program understands the following environment variables:

|   Variable Name     |   Type             |   Argument (if any)          |
|-------------------------------------------------------------------------|
VIS_BOWSER_NO_AUDIO     (boolean)           -q
    If set to any value, do not load audio.

VIS_BOWSER_ISOLATE      (track number)      --isolate=num
    If set to a track number (1..8), only schedule that track. If possible,
    only play the audio for that track as well (unless VIS_BOWSER_NO_AUDIO
    is set).

VIS_BOWSER_SKIP_INTRO   (boolean)           --si
    If set to any value, seek to offset 18 seconds 87 milliseconds, which
    is the start of the main track.

VIS_BOWSER_SKIP_TO      (milliseconds)      --st=ms
    If set to a numeric value, seek both the visualization and the audio to
    that offset in milliseconds.

VIS_BOWSER_FRAME_TWEAK  (frame number)      --ft=frame
    If set to a numeric value, seek only the visualization and not the
    audio to that offset in frames (running at Vis.FPS_LIMIT frames per
    second).

VIS_BOWSER_MS_TWEAK     (milliseconds)      --mst=ms
    If set to a numeric value, seek only the visualization and not the
    audio to that offset in milliseconds.

VIS_BOWSER_EXIT_MS      (milliseconds)      --exit=ms
    If set to a numeric value, exit at precisely this time in milliseconds.
    If un-set, exit after the final scheduled emit has been emitted.

VIS_BOWSER_DUMP_TRACKn  (boolean)           --dump=n    (n = track number)
    If set to any value, print out the entire schedule of track n. The
    value n is an integer between 1 and 8, inclusive.

VIS_DEBUG               (boolean/integer)   --debug
    If set to any value, enable debugging output.

VIS_HELP                (boolean)           --help
    If set to any value, print out the understood environment variables
    (this list) and some useful offsets in milliseconds to key positions
    in the visualization.

The following offsets may be helpful for the environment variables above:

 1223 (milliseconds)    36 (frames)     (at 30 frames per second)
    Track 1 intro, part 1.
 7108 (milliseconds)    213 (frames)    (at 30 frames per second)
    Track 1 intro, part 2.
13422 (milliseconds)    402 (frames)    (at 30 frames per second)
    Track 1 intro, part 3.
13675 (milliseconds)    410 (frames)    (at 30 frames per second)
    Track 3 intro, part 4, start of top and bottom fire.
18087 (milliseconds)    542 (frames)    (at 30 frames per second)
    Start of main song; VIS_BOWSER_SKIP_INTRO seeks to precisely this
    offset.
19211 (milliseconds)    576 (frames)    (at 30 frames per second)
    Start of track 4's repeating sections.
]]

Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")
os = require("os")
Debug = VisUtil.Debug

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

-- Configuration
local function EstablishConfig()
    Config = {}
    Config[#Config+1] = {
        'VIS_BOWSER_NO_AUDIO',
        '-q',
        'nil',
        "do not play music"
    }
    Config[#Config+1] = {
        'VIS_BOWSER_ISOLATE',
        '--isolate',
        'number',
        "play only this track"
    }
    Config[#Config+1] = {
        'VIS_BOWSER_SKIP_INTRO',
        '--si',
        'nil',
        "skip the intro"
    }
    Config[#Config+1] = {
        'VIS_BOWSER_SKIP_TO',
        '--st',
        'number',
        "skip to this offset in msec"
    }
    Config[#Config+1] = {
        'VIS_BOWSER_FRAME_TWEAK',
        '--ft',
        'number',
        "seek to this frame"
    }
    Config[#Config+1] = {
        'VIS_BOWSER_MS_TWEAK',
        '--mst',
        'number',
        "seek to this msec"
    }
    Config[#Config+1] = {
        'VIS_BOWSER_EXIT_MS',
        '--exit',
        'number',
        "exit after this number of msec"
    }
    for i = TRACK_1, TRACK_8 do
        Config[#Config+1] = {
            'VIS_BOWSER_DUMP_TRACK'..i,
            '--dump-'..i,
            'nil',
            'dump track '..i..'\'s schedule'
        }
    end
    Config[#Config+1] = {
        'VIS_DEBUG',
        '--debug',
        'nil',
        "enable debugging"
    }
    Config[#Config+1] = {
        'LUA_DEBUG',
        '--debug',
        'nil',
        "enable debugging"
    }
    Config[#Config+1] = {
        'DEBUG',
        '--debug',
        'nil',
        "enable debugging"
    }
    local parser = VisUtil.Args:new()
    for _, argspec in pairs(Config) do
        parser:add(argspec[2], argspec[3], argspec[4])
        parser:add_env(argspec[1], argspec[3], argspec[4])
        parser:link_arg_env(argspec[2], argspec[1])
    end
    local args, envs, err = parser:parse()
    if err ~= nil then error(err) end
    --if envs['VIS_HELP'] ~= nil then
    --    print(VIS_BOWSER_HELP_STRING)
    --end
    return args, envs
end

Arg, Env = EstablishConfig()

-- Important timekeeping functions
function now(track)
    return TrackTimes[track]
end

function adv(track, length)
    return set(track, TrackTimes[track] + length)
end

function set(track, offset)
    TrackAdvances[track] = TrackAdvances[track] + 1
    if offset < TrackTimes[track] then
        error("Should not be going back in time!")
    end
    TrackTimes[track] = offset
    TrackEndTime = math.max(TrackTimes[track], TrackEndTime)
    return now(track)
end

-- Select which file to play and load it
if Env['VIS_BOWSER_NO_AUDIO'] == nil then
    local isolate = Env['VIS_BOWSER_ISOLATE']
    local result = nil
    if isolate ~= nil then
        result = Vis.audio(Vis.flist, 0, "media/bowser-track-"..isolate..".mp3")
        if result == nil then
            print("Failed to load isolated track, loading full track")
        end
    end
    if result == nil then
        Vis.audio(Vis.flist, 0, "media/bowser-full.mp3")
    end
end

-- Position helpers for placing emits at precise locations
function W(num, denom) return Vis.WIDTH * num / denom end
W_1_2 = W(1,2) -- 400
W_1_3 = W(1,3) -- 266.67
W_2_3 = W(2,3) -- 533.33
W_1_4 = W(1,4) -- 200
W_3_4 = W(3,4) -- 600
W_1_5 = W(1,5) -- 160
W_2_5 = W(2,5) -- 320
W_3_5 = W(3,5) -- 480
W_4_5 = W(4,5) -- 640
W_1_6 = W(1,6) -- 133.33
W_5_6 = W(5,6) -- 666.67
W_1_8 = W(1,8) -- 100

function H(num, denom) return Vis.HEIGHT * num / denom end
H_1_2 = H(1,2) -- 300
H_1_3 = H(1,3) -- 200
H_2_3 = H(2,3) -- 400
H_1_4 = H(1,4) -- 150
H_3_4 = H(3,4) -- 450
H_1_5 = H(1,5) -- 120
H_2_5 = H(2,5) -- 240
H_3_5 = H(3,5) -- 360
H_4_5 = H(4,5) -- 480
H_1_6 = H(1,6) -- 100
H_5_6 = H(5,6) -- 500
H_1_8 = H(1,8) --  75

-- Helper functions

-- Transparently wrap a function and provide debugging messages
function debug_wrap(fn, fnname)
    function wrapper(...)
        if fnname == nil then
            ar = debug.getinfo(fn)
            fnname = ("%s:%s:%s"):format(ar.name, ar.namewhat, ar.source)
        end
        local value = fn(...)
        Debug(Vis.DEBUG > Vis.DEBUG_DEBUG and Debug.L_TRACEBACK or 0,
              "%s(%s) -> %s", fnname, VisUtil.strobject(...), value)
        return value
    end
    return wrapper
end

-- Load a specific track's configuration, unless instructed not to
local function dotrack(track)
    local isolate = Env['VIS_BOWSER_ISOLATE']
    if isolate == nil or isolate == tostring(track) then
        dofile("lua/bowser/track"..track..".lua")
    end
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
for i = TRACK_1,TRACK_8 do
    local Tn = {} -- Track namespace
    _G['T'..i] = Tn
    Tn.ScheduleIndex = 0
    -- This kind of function binding makes the functions lose their names. Try
    -- to find a way to preserve function names so debugging works better.
    Tn.NextSchedule = (function(track, track_num)
        function Tn_NextSchedule()
            track.ScheduleIndex = track.ScheduleIndex + 1
            if track.ScheduleIndex > #track.SCHEDULE then
                print(("SI %d > %d"):format(track.ScheduleIndex, #track.SCHEDULE))
                print(debug.traceback())
                return track.SCHEDULE[#track.SCHEDULE]
            end
            return track.SCHEDULE[track.ScheduleIndex]
        end
        return Tn_NextSchedule
    end)(Tn, i)
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
for i = TRACK_1,TRACK_8 do
    dotrack(i)
    local tn = _G['T'..i]
    if Env['VIS_BOWSER_DUMP_TRACK'..i] ~= nil and tn.SCHEDULE ~= nil then
        local si = tn.ScheduleIndex
        print(("T%d.ScheduleIndex = %d"):format(i, si))
        print(("T%d.SCHEDULE = {"):format(i))
        print(("    -- msec, msec, -- idx, dt"))
        for i = 1,#tn.SCHEDULE,2 do
            str = "    %s, %s, -- [%s] %s msec"
            if si == i or si == i+1 then
                str = "    %s, %s, -- [%s] %s msec << ScheduleIndex: " .. si
            end
            s1, s2 = tn.SCHEDULE[i], tn.SCHEDULE[i+1]
            print(str:format(s1, s2, i, (s2 or s1) - s1))
        end
        print("}")
    end
end

-- Figure out when we exit
local exit_time = tonumber(Env['VIS_BOWSER_EXIT_MS']) or TrackEndTime
Debug("Exiting at %d ms", exit_time)
Vis.exit(Vis.flist, exit_time)

--[[ FIXME!!
--
--  Track four has the emit-in-a-circle design which looks awful
--  Track one has that 2x2 box design which looks awful
--
--]]
