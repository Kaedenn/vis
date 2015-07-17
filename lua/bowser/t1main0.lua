Emit = require('emit')

-- MAIN TRACK: PART ZERO: INTRO TO MAIN TRACK

local exi = 1
local ex = {W_1_4, W_1_3}

local eyi = 1
local ey = {H_1_6, H_1_4, H_1_3, H_1_2, -H_1_2, -H_1_3, -H_1_4, -H_1_6}

local function next_ex()
    local x1, x2 = ex[exi], -ex[exi]
    exi = (exi + 1) % #ex + 1
    return x1, x2
end

local function next_ey()
    local y1, y2 = ey[eyi], ey[eyi]
    eyi = (eyi + 1) % #ey + 1
    return y1, y2
end

local function emit_pair(e, ds)
    e2 = Emit:copy(e)
    if ds ~= nil then e2:ds(ds) end
    x1, x2 = next_ex()
    y1, y2 = next_ey()
    j = T1.next()
    k = T1.next()
    for now = j,k,Vis.frames2msec(1) do
        e2:when(now)
        e2:center(x1, y1)
        e2:emit()
        e2:center(x2, y2)
        e2:emit()
    end
end

local circle = Emit:new()
circle:radius(1)
circle:ds(2)
circle:life(SECOND/2)
circle:color(0, .4, .8, 0, .2, 0)
circle:count(100)
circle:theta(math.pi, math.pi)

ds = {2, 1.9, 1.7, 1.5, 1, 1, 1, 1}
for dsi = 1,8 do
    dsv = ds[dsi]
    emit_pair(circle, dsv)
end

