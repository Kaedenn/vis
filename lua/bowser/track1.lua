do

debug = require("debug")

T1.SCHEDULE = {
    -- INTRO: PART ONE
     1223,          -- Circle
     1594,          -- Circle
     1960,          -- Circle
     2328,  3393,   -- Repeating Circle
     3423,          -- Circle
     3676,          -- Circle
     4227,          -- Circle (quieter)
     4474,          -- Circle (quieter)
     5027,          -- Circle (much quieter)
     5272,          -- Circle (much quieter)
    -- INTRO: PART TWO
     7108,          -- Circle
     7477,          -- Circle
     7842,          -- Circle
     8209,  9281,   -- Repeating Circle
     9313,          -- Circle
     9560,          -- Circle
     9804, 10261,   -- Line
    10602, 11061,   -- Line
    11400, 11859,   -- Line
    -- INTRO: PART THREE
    13422,          -- Circle
    13791,          -- Circle
    14155,          -- Circle
    14526, 15596,   -- Repeating Circle
    15631,          -- Circle
    15873,          -- Circle
    16426,          -- Circle (quieter)
    16671,          -- Circle (quieter)
    17220,          -- Circle (much quieter)
    17469,          -- Circle (much quieter)
    -- MAIN TRACK: PART ZERO
    18087, 18191,
    18208, 18313,
    18334, 18439,
    18459, 18687,
    18709, 18815,
    18835, 18941,
    18957, 19066,
    19084, 19190
}

T1.SCHEDULE_MAIN1_REPEATS = {
    19211, 23219, 27227, 31235
}

T1.SCHEDULE_MAIN1_PART1 = {
       0,  103, -- note 1
     123,  226, -- note 2
     246,  349, -- note 3
     369,  472, -- note 4
     492,  595, -- note 5
     615,  718, -- note 6
     738,  841, -- note 7
     861,  964, -- note 8
     984, 1087, -- note 9
    1107, 1210, -- note 10
    1230, 1333, -- note 11
    1353, 1456, -- note 12
    1476, 1579, -- note 13
    1599, 1702, -- note 14
    1722, 1825, -- note 15
    1845, 1948, -- note 16
}

T1.SCHEDULE_MAIN1_PART2 = {
    2002, 2234,
    2251, 2358,
    2375, 2731,
    2753, 2984,
    3002, 3111,
    3126, 3484,
    3501, 3609,
    3626, 3735,
    3752, 3862,
    3874, 3982
}

for _,i in pairs(T1.SCHEDULE_MAIN1_REPEATS) do
    for n,j in pairs(T1.SCHEDULE_MAIN1_PART1) do
        table.insert(T1.SCHEDULE, i + j)
    end
    for n,j in pairs(T1.SCHEDULE_MAIN1_PART2) do
        table.insert(T1.SCHEDULE, i + j)
    end
end

-- MAIN TRACK: PART ONE: 20008ms
-- m1p1 total: 20008ms, repeat 4 times
-- m1p1 each: 5002ms
-- 19211, 21186, -- m1p1

--[[
--  Main track part one has sixteen uniform tones
--      tone: 103ms
--      gap: 20ms
--      total: 123ms * 16 = duration of m1p1
--  Main track part two has a b c a b c 1 2 3 4
--  length: b < a < c
--  pitch: b < a < c
--  pitch: 1 > 2, 1,2 < 3 < 4
--]]

T1.REPEAT = {}
T1.REPEAT.M1 = 0
T1.REPEAT.M1_SHORT = 4

dofile("lua/bowser/t1intro.lua")
dofile("lua/bowser/t1main0.lua")
while T1.REPEAT.M1 < T1.REPEAT.M1_SHORT do
    dofile("lua/bowser/t1main1.lua")
    T1.REPEAT.M1 = T1.REPEAT.M1 + 1
end

end
