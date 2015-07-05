do

T1 = {}
T1.SCHEDULE_TRACK1 = {
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
    -- MAIN TRACK: PART ONE
    18087, 18191,
    18208, 18313,
    18334, 18439,
    18459, 18687,
    18709, 18815,
    18835, 18941,
    18957, 19066,
    19084, 19190,
    -- MAIN TRACK: PART TWO
    19172, 39133
}

T1.ScheduleIndex = 0;

function T1.NextSchedule()
    T1.ScheduleIndex = T1.ScheduleIndex + 1
    if T1.ScheduleIndex > #T1.SCHEDULE_TRACK1 then
        print("ScheduleIndex " .. T1.ScheduleIndex .. " is greater than " ..
              #T1.SCHEDULE_TRACK1)
        return T1.SCHEDULE_TRACK1[#T1.SCHEDULE_TRACK1]
    end
    return T1.SCHEDULE_TRACK1[T1.ScheduleIndex]
end

function T1.emit_circle(start, x, y, ds)
    local MAXLIFE = SECOND/3
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, x or Vis.WIDTH/3, y or Vis.HEIGHT*2/3)
    et.ds = ds or 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, 0, 100, 200, 0, 50, 0);
    et.count = 100
    et.when = start
    et.theta = math.pi
    et.utheta = math.pi
    VisUtil.emit_table(et)
end

dofile("lua/bowser/t1intro.lua")
dofile("lua/bowser/t1main1.lua")

end
