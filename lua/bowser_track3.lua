do

local SCHEDULE_TRACK3 = {
    0, 5848,
    5893, 12043,
    12210, 13630,
    13675, 14606,
    14653, 15596,
    15631, 16085,
    16120, 16574,
    16611, 17068,
    17102, 17561,
    17595, 18048
}

local ScheduleIndex = 0;

local function NextSchedule()
    ScheduleIndex = ScheduleIndex + 1
    if ScheduleIndex > #SCHEDULE_TRACK3 then
        print("ScheduleIndex " .. ScheduleIndex .. " is greater than " ..
              #SCHEDULE_TRACK3)
        return SCHEDULE_TRACK3[#SCHEDULE_TRACK3]
    end
    return SCHEDULE_TRACK3[ScheduleIndex]
end

function set_emit_left(et)
    VisUtil.center_emit_table(et, 0, 0, 0, Vis.HEIGHT)
    et.theta = 0
end

function set_emit_right(et)
    VisUtil.center_emit_table(et, Vis.WIDTH, 0, 0, Vis.HEIGHT)
    et.theta = math.pi
end

function set_emit_top(et)
    VisUtil.center_emit_table(et, 0, 0, Vis.WIDTH, 0)
    et.theta = math.pi/2
end

function set_emit_bottom(et)
    VisUtil.center_emit_table(et, 0, Vis.HEIGHT, Vis.WIDTH, 0)
    et.theta = 3*math.pi/2
end

function make_emit_tables_for_level(level)
    local results = {}
    local et = VisUtil.make_emit_table()
    local ds_tab = {2, 3, 4, 5}
    local uds_tab = {0, .2, .25, .3, .4, .5}
    local life_tab = {200, 210, 220, 230, 240, 250}
    local rgb_tab = {{.8, .2, .2, .2, .1, .1},
                     {.9, 0, 0, .1, 0, 0}}
    local radius_tab = {1, 1, 2, 2, 2, 3, 3, 3, 4, 5}
    et.count = 200
    et.ds = ds_tab[level] or ds_tab[#ds_tab]
    et.uds = uds_tab[level] or uds_tab[#uds_tab]
    et.life = life_tab[level] or life_tab[#life_tab]
    VisUtil.color_emit_table_v(et, rgb_tab[level] or rgb_tab[#rgb_tab])
    et.utheta = 0.2

    set_emit_left(et)
    table.insert(results, VisUtil.copy_table(et))
    set_emit_right(et)
    table.insert(results, VisUtil.copy_table(et))
    if level > 5 then
        set_emit_top(et)
        et.x = Vis.WIDTH / 2
        et.ux = Vis.WIDTH / 10 * (level - 5)
        table.insert(results, VisUtil.copy_table(et))
        set_emit_bottom(et)
        et.x = Vis.WIDTH / 2
        et.ux = Vis.WIDTH / 10 * (level - 5)
        table.insert(results, VisUtil.copy_table(et))
    end
    return results
end

for level = 1, 10 do
    settime(TRACK_3, NextSchedule())
    j = now(TRACK_3)
    settime(TRACK_3, NextSchedule())
    while j < now(TRACK_3) do
        local ets = make_emit_tables_for_level(level)
        for k = 1, #ets do
            ets[k].when = j
            VisUtil.emit_table(ets[k])
        end
        j = j + Vis.frames2msec(1)
    end
end

-- We're only doing the intro for now
return

end
