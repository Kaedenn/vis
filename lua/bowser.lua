
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

VIS_BOWSER_NO_AUDIO     (boolean)
    If set to any value, do not load audio.

VIS_BOWSER_ISOLATE      (track number)
    If set to a track number (1..8), only schedule that track. If possible,
    only play the audio for that track as well (unless VIS_BOWSER_NO_AUDIO
    is set).

VIS_BOWSER_SKIP_INTRO   (boolean)
    If set to any value, seek to offset 18 seconds 87 milliseconds, which
    is the start of the main track.

VIS_BOWSER_SKIP_TO      (milliseconds)
    If set to a numeric value, seek both the visualization and the audio to
    that offset in milliseconds.

VIS_BOWSER_FRAME_TWEAK  (frame number)
    If set to a numeric value, seek only the visualization and not the
    audio to that offset in frames (running at Vis.FPS_LIMIT frames per
    second).

VIS_BOWSER_MS_TWEAK     (milliseconds)
    If set to a numeric value, seek only the visualization and not the
    audio to that offset in milliseconds.

VIS_BOWSER_EXIT_MS      (milliseconds)
    If set to a numeric value, exit at precisely this time in milliseconds.
    If un-set, exit after the final scheduled emit has been emitted.

VIS_BOWSER_DUMP_TRACKn  (boolean, n = track number)
    If set to any value, print out the entire schedule of track n. The
    value n is an integer between 1 and 8, inclusive.

VIS_DEBUG               (boolean/integer)
    If set to any value, enable debugging output.

VIS_HELP                (boolean)
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
Env['VIS_BOWSER_NO_AUDIO'] = '<bool>'
Env['VIS_BOWSER_ISOLATE'] = '<track-number>'
Env['VIS_BOWSER_SKIP_INTRO'] = '<bool>'
Env['VIS_BOWSER_SKIP_TO'] = '<offset>'
Env['VIS_BOWSER_FRAME_TWEAK'] = '<frame-offset>'
Env['VIS_BOWSER_MS_TWEAK'] = '<offset>'
Env['VIS_BOWSER_EXIT_MS'] = '<offset>'
Env['VIS_DEBUG'] = '<bool>'
Env['VIS_HELP'] = '<bool>'

for t = 1,TRACKS do
    Env['VIS_BOWSER_DUMP_TRACK'..t] = '<bool>'
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
    local isolate = Env['VIS_BOWSER_ISOLATE']
    local result = nil
    if isolate ~= nil then
        result = Vis.audio(Vis.flist, 0, "media/bowser-track-"..isolate..".mp3")
    end
    if result == nil then
        if isolate ~= nil then
            print("Failed to load isolated track, loading full track")
        end
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
        local s = "%s(%s) -> %s"
        local value = fn(...)
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
    Tn.NextSchedule = (function(track, track_num)
        return function()
            track.ScheduleIndex = track.ScheduleIndex + 1
            if track.ScheduleIndex > #track.SCHEDULE then
                print("ScheduleIndex " .. track.ScheduleIndex ..
                      " is greater than " ..  #track.SCHEDULE)
                print(debug.traceback())
                return track.SCHEDULE[#track.SCHEDULE]
            end
            return track.SCHEDULE[track.ScheduleIndex]
        end
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
    print(VIS_BOWSER_HELP_STRING)
end

-- Figure out when we exit
Vis.exit(Vis.flist, tonumber(Env['VIS_BOWSER_EXIT_MS']) or TrackEndTime)

