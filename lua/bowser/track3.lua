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

function T3.make_emits_for_level(level)
    local results = {}
    local e = Emit:new()
    local ds_tab = {2, 2.5, 3, 3.5}
    local life_tab = {200, 210, 220, 230, 240, 250}
    local rgb_tab = {{.8, .2, .2, .2, .1, .1},
                     {.9, 0, 0, .1, 0, 0}}
    e:count(200)
    e:radius(2)
    e:ds(ds_tab[level] or ds_tab[#ds_tab], et.ds / 2 - et.ds / 5)
    e:life(life_tab[level] or life_tab[#life_tab])
    e:color(rgb_tab[level] or rgb_tab[#rgb_tab])

    e:center(0, 0, 0, Vis.HEIGHT)
    e:theta(0, 0.8)
    table.insert(results, e:clone())
    e:center(Vis.WIDTH, 0, 0, Vis.HEIGHT)
    e:theta(math.pi, 0.8)
    table.insert(results, e:clone())
    if level > 3 then
        e:center(Vis.WIDTH/2, 0, Vis.WIDTH / 14 * (level - 3), 0)
        e:theta(math.pi/2, 0.8)
        table.insert(results, e:clone())
        e:center(Vis.WIDTH/2, Vis.HEIGHT, Vis.WIDTH / 14 * (level - 3), 0)
        e:theta(3*math.pi/2, 0.8)
        table.insert(results, e:clone())
    end
    return results
end

-- INTRO: ALL TEN PARTS
local level
for level = 1,10 do
    local ets = T3.make_emits_for_level(level)
    local j = T3.next()
    local k = T3.next()
    while j < k do
        for _,e in pairs(ets) do
            e:emit_at(j)
        end
        j = j + Vis.frames2msec(1)
    end
end

-- We're only doing the intro for now
return

end
