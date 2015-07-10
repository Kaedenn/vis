do

T3.SCHEDULE = {
    -- INTRO
    0, 5848,
    5893, 12043,
    12210, 13630,
    13675, 14606,
    14653, 15596,
    15631, 16085,
    16120, 16574,
    16611, 17068,
    17102, 17561,
    17595, 18048,
    -- MAIN SONG
}

function T3.make_emit_tables_for_level(level)
    local results = {}
    local et = VisUtil.make_emit_table()
    local ds_tab = {2, 2.5, 3, 3.5}
    local life_tab = {200, 210, 220, 230, 240, 250}
    local rgb_tab = {{.8, .2, .2, .2, .1, .1},
                     {.9, 0, 0, .1, 0, 0}}
    et.count = 200
    et.radius = 2
    et.ds = ds_tab[level] or ds_tab[#ds_tab]
    et.uds = et.ds / 2 - et.ds / 5
    et.life = life_tab[level] or life_tab[#life_tab]
    VisUtil.color_emit_table_v(et, rgb_tab[level] or rgb_tab[#rgb_tab])
    et.utheta = 0.8

    Emits.set_emit_left(et)
    table.insert(results, VisUtil.copy_table(et))
    Emits.set_emit_right(et)
    table.insert(results, VisUtil.copy_table(et))
    if level > 3 then
        Emits.set_emit_top(et)
        et.x = Vis.WIDTH / 2
        et.ux = Vis.WIDTH / 14 * (level - 3)
        table.insert(results, VisUtil.copy_table(et))
        Emits.set_emit_bottom(et)
        et.x = Vis.WIDTH / 2
        et.ux = Vis.WIDTH / 14 * (level - 3)
        table.insert(results, VisUtil.copy_table(et))
    end
    return results
end

-- INTRO: ALL TEN PARTS
local level
local k
for level = 1, 10 do
    settime(TRACK_3, T3.NextSchedule())
    local j = now(TRACK_3)
    settime(TRACK_3, T3.NextSchedule())
    while j < now(TRACK_3) do
        local ets = T3.make_emit_tables_for_level(level)
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
