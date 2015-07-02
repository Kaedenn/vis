do

--[[
Intro: Part One
1223, 1389, Circle
1594, 1760, Circle
1960, 2128, Circle
2328, 3393, Repeating Circle
3432, 3640, Circle
3676, 3886, Circle
4227, 4438, Circle (quieter)
4474, 4683, Circle (quieter)
5027, 5233, Circle (much quieter)
5272, 5478, Circle (much quieter)

7108, 7278, Circle
7477, 7646, Circle
7842, 8016, Circle
8209, 9281, Repeating Circle
9313, 9527, Circle
9560, 9774, Circle
9804, 10261, Line
10602, 11061, Line
11400, 11859, Line

13422, 13593, Circle
13791, 13962, Circle
14155, 14329, Circle
14526, 15596, Repeating Circle
15631, 15839, Circle
15873, 16085, Circle
16426, 16636, Circle (quieter)
16671, 16879, Circle (quieter)
17220, 17436, Circle (much quieter)
17469, 17681, Circle (much quieter)
--]]
local SCHEDULE_TRACK1 = {
    -- INTRO: PART ONE
    1223,
    1594,
    1960,
    2328, 3393,
    3423,
    3676,
    4227,
    4474,
    5027,
    5272,
    -- INTRO: PART TWO
    7108,
    7477,
    7842,
    8209, 9281,
    9313,
    9560,
    9804, 10261,
    10602, 11061,
    11400, 11859,
    -- INTRO: PART THREE
    13422,
    13791,
    14155,
    14526, 15596,
    15631,
    15873,
    16426,
    16671,
    17220,
    17469,
    -- MAIN TRACK: PART ONE
    18084
}

local ScheduleIndex = 0;

local function NextSchedule()
    ScheduleIndex = ScheduleIndex + 1
    if ScheduleIndex > #SCHEDULE_TRACK1 then
        print("ScheduleIndex " .. ScheduleIndex .. " is greater than " ..
              #SCHEDULE_TRACK1)
        return SCHEDULE_TRACK1[#SCHEDULE_TRACK1]
    end
    return SCHEDULE_TRACK1[ScheduleIndex]
end

function emit_circle(start, x, y, r, g, b, ur, ug, ub)
    local MAXLIFE = SECOND/3
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, x or Vis.WIDTH/3, y or Vis.HEIGHT*2/3)
    et.ds = 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0);
    et.count = 100
    et.when = start
    et.theta = math.pi
    et.utheta = math.pi
    VisUtil.emit_table(et)
end

-- INTRO: PART ONE
settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_4, H_5_6); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_3_4, H_5_6); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, NextSchedule())
j = now(TRACK_1)
settime(TRACK_1, NextSchedule())
while j < now(TRACK_1) do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
settime(TRACK_1, NextSchedule())

emit_circle(now(TRACK_1), W_1_2, H_2_3); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, NextSchedule())

emit_circle(now(TRACK_1), W_1_2, H_2_3)
settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_2, H_3_4)
settime(TRACK_1, NextSchedule())

emit_circle(now(TRACK_1), W_1_2, H_2_3); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, NextSchedule())

-- INTRO: PART TWO
emit_circle(now(TRACK_1), W_1_4, H_5_6); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_3_4, H_5_6); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, NextSchedule())
j = now(TRACK_1)
settime(TRACK_1, NextSchedule())
while j < now(TRACK_1) do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_4, H_5_6); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_3_4, H_5_6); settime(TRACK_1, NextSchedule())

j = now(TRACK_1)
settime(TRACK_1, NextSchedule())
while j < now(TRACK_1) do
    emit_line_v(j, W_1_2)
    j = j + 33
end
settime(TRACK_1, NextSchedule())

j = now(TRACK_1)
settime(TRACK_1, NextSchedule())
while j < now(TRACK_1) do
    emit_line_v(j, W_1_2)
    j = j + 33
end
settime(TRACK_1, NextSchedule())

j = now(TRACK_1)
settime(TRACK_1, NextSchedule())
while j < now(TRACK_1) do
    emit_line_v(j, W_1_2)
    j = j + 33
end
settime(TRACK_1, NextSchedule())

-- INTRO: PART THREE
emit_circle(now(TRACK_1), W_1_4, H_5_6); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_3_4, H_5_6); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, NextSchedule())
j = now(TRACK_1)
settime(TRACK_1, NextSchedule())
while j < now(TRACK_1) do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
settime(TRACK_1, NextSchedule())

emit_circle(now(TRACK_1), W_1_2, H_2_3); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, NextSchedule())

emit_circle(now(TRACK_1), W_1_2, H_2_3); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, NextSchedule())

emit_circle(now(TRACK_1), W_1_2, H_2_3); settime(TRACK_1, NextSchedule())
emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, NextSchedule())

end
