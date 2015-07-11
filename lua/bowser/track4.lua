do

debug = require("debug")
table = require("table")

Emit = require("emit")

--[[ TRACK 4 STRUCTURE
--
--  Track 4 begins where the intro ends: 18087.
--
--  It starts with eight notes that do not repeat.
--      T4.M0P0.1 = 18087: 8 notes
--  Next comes sixteen notes that do not repeat.
--      T4.M1P0.2 = 19211: 16 notes
--  Next comes ten notes that do not repeat.
--      T4.M1P0.3 = 21213: 10 notes
--  Next comes sixteen notes (same schedule as T4.M1P0.2) that do repeat.
--      T4.M1P1 = 23219: 16 notes
--  Next comes ten notes (same schedule as T4.M1P0.3) that do repeat.
--      T4.M1P2 = 25218: 10 notes
--
--  Repeats are implemented if and only if GENERATE1 has more than eight
--  entries.
--]]

T4.SCHEDULE = {
    -- MAIN TRACK: PART INTRO
    -- Same schedule as T1 MAIN TRACK: PART ZERO
    18087, 18191,
    18208, 18313,
    18334, 18439,
    18459, 18687,
    18709, 18815,
    18835, 18941,
    18957, 19066,
    19084, 19190,
}

T4.SCHEDULE_GENERATE1 = {
    -- MAIN TRACK: PART INTRO 2
    -- Starts at T1 MAIN 1 PART 1 (first repeat)
    -- Has the same note structure
    19211, 21195,
    -- MAIN TRACK: PART INTRO 3
    -- Starts at T1 MAIN 1 PART 2 (first repeat)
    -- Has the same note structure
    21213, 23193,
    -- MAIN TRACK: PART ONE
    -- Starts near T1 MAIN 1 PART 1 (second repeat)
    -- Has the same note structure
    23219, 25198,
    -- MAIN TRACK: PART TWO
    -- Starts near T1 MAIN 1 PART 2 (second repeat)
    -- Has the same note structure
    25218, 27337,
}

T4.SCHEDULE_MAIN1_INTRO3 = {
    21213, 21449, -- m1p2.1
    21462, 21573, -- m1p2.2
    21586, 21948, -- m1p2.3
    21963, 22197, -- m1p2.4
    22212, 22324, -- m1p2.5
    22338, 22699, -- m1p2.6
    22714, 22821, -- m1p2.7
    22837, 22947, -- m1p2.8
    22965, 23193, -- m1p2.9
}

T4.peek = function()
    return T4.SCHEDULE[T4.ScheduleIndex + 1] - T4.SCHEDULE[T4.ScheduleIndex]
end

-- Track 1 does not have an offset key for MAIN 1 PART 2; it comes
-- directly after MAIN 1 PART 1. Track 4 *does*, however, so take that
-- into account by subtracting out the start of the first note.
T4.T1M1P2_ERROR = T1.SCHEDULE_MAIN1_PART2[1]

for note,ms in pairs(T1.SCHEDULE_MAIN1_PART1) do
    table.insert(T4.SCHEDULE, T4.SCHEDULE_GENERATE1[1] + ms)
end
for note,ms in pairs(T4.SCHEDULE_MAIN1_INTRO3) do
    table.insert(T4.SCHEDULE, ms)
end

local e = Emit:new()
e:count(1000)
e:radius(2)
e:ds(4)
e:life(SECOND/3)
e:color(0.7, 0.2, 0.3, 0.2, 0.1, 0.1)
e:theta(math.pi, math.pi)
e:blender(Vis.GEOMETRIC_BLEND)

-- M1P0.1
for i = 1,16,2 do
    T4.set(T4.NextSchedule())
    e:ds(4,2)
    e:life(T4.peek(), T4.peek()/2)
    local e2 = e:clone()
    e:center(10, H(i, 18))
    e2:center(Vis.WIDTH - 10, H(i, 18))
    for j = T4.now(),T4.now()+T4.peek(),Vis.frames2msec(1) do
        e:when(j)
        e:emit()
        e2:when(j)
        e2:emit()
    end
    T4.set(T4.NextSchedule())
end

-- M1P0.2
e:ds(4, 2)
for i = 1,32,2 do
    T4.set(T4.NextSchedule())
    dist = W_1_4
    angle = 2 * math.pi / 32 * i + (math.pi * (i % 2))
    e:center(W_1_2 + dist * math.cos(angle), H_1_2 + dist * math.sin(angle))
    e:life(T4.peek(), T4.peek()/2)
    for j = T4.now(),T4.now()+T4.peek(),Vis.frames2msec(1) do
        e:when(j)
        e:emit()
    end
    T4.set(T4.NextSchedule())
end

-- M1P0.3
e:ds(8,4)
e:count(2000)
for i = 1,#T4.SCHEDULE_MAIN1_INTRO3,2 do
    T4.set(T4.NextSchedule())
    e:center(W_1_2, Vis.HEIGHT - H_1_4/#T4.SCHEDULE_MAIN1_INTRO3*(i+1))
    e:life(math.max(T4.peek()/10, Vis.frames2msec(3)))
    for j = T4.now(),T4.now()+T4.peek(),Vis.frames2msec(1) do
        e:when(j)
        e:emit()
    end
    T4.set(T4.NextSchedule())
end

-- END FILE

end
