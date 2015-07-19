do

debug = require("debug")
table = require("table")

Emit = require("emit")

T8.SCHEDULE = {
    -- MAIN TRACK PART ONE
    -- 23219, 27332, -- m8p1
    23219, 23327, -- m8p1.1
    23341, 23447, -- m8p1.2
    23466, 23572, -- m8p1.3
    23591, 23697, -- m8p1.4
    23717, 23822, -- m8p1.5
    23840, 23948, -- m8p1.6
    23966, 24071, -- m8p1.7
    24091, 24197, -- m8p1.8
    24216, 24323, -- m8p1.9
    24341, 24446, -- m8p1.10
    24468, 24572, -- m8p1.11
    24591, 24698, -- m8p1.12
    24716, 24821, -- m8p1.13
    24841, 24947, -- m8p1.14
    24968, 25072, -- m8p1.15
    25090, 25198, -- m8p1.16
    -- Start of second part
    25218, 25447, -- m8p1.17
    25469, 25574, -- m8p1.18
    25590, 25948, -- m8p1.19
    25968, 26200, -- m8p1.20
    26218, 26325, -- m8p1.21
    26341, 26699, -- m8p1.22
    26717, 26828, -- m8p1.23
    26840, 26949, -- m8p1.24
    26969, 27076, -- m8p1.25
    27090, 27202, -- m8p1.26
    27225, 27332, -- m8p1.27
    -- TRACK 8 TONE: 2.407 seconds
    27332, 29739, -- m8p2
    -- TRACK 8 TONE FADE: 0.417 seconds (~2.824 sec, 84 frames for whole tone)
    29739, 30156, -- m8p2-fade
    -- TRACK 8 NOTE
    30982, 31087, -- m8p3
    -- TRACK 8 NOTE
    31105, 31214, -- m8p4
    -- TRACK 8 TONE
    31237, 32967, -- m8p5
    -- TRACK 8 NOTE
    32984, 33089, -- m8p6
    -- TRACK 8 NOTE
    33105, 33216, -- m8p7
    -- TRACK 8 TONE
    33234, 34217, -- m8p8
}

local e = Emit:new()
e:count(100)
e:radius(2)
e:ds(3, 0.5)
e:theta(math.pi, math.pi)
e:life(SECOND/4)

e:center(W_1_3, H_1_2, 0, 0)
for i = 1,16 do
    start = T8.next()
    stop = T8.next()
    for j = start,stop,Vis.frames2msec(1) do
        e:emit_at(j)
    end
    e:set('x', -e:get('x'))
end

e:center(W_1_2, H(1,20), 0, 0)
for i = 1,8 do
    start = T8.next()
    stop = T8.next()
    for j = start,stop,Vis.frames2msec(1) do
        e:emit_at(j)
    end
    e:center(W_1_2, H(i+1, 20), 0, 0)
end

-- nom nom
-- There's too much visual stimulation, so skip these three next notes
for i = 1,6 do T8.next() end

-- The tone
local tonestart = T8.next()
local tonestop = T8.next()
local tonefadestart = T8.next()
local tonefadestop = T8.next()
local bgc = 0
fc = math.floor((tonefadestop - tonestart)/Vis.frames2msec(1))
fi = 0
for j = tonestart,tonefadestop,Vis.frames2msec(1) do
    if fi < 5 then
        bgc = 0.05 * 2*fi
    elseif fi < fc-10 then
        bgc = 0.50
    else
        bgc = 0.05 * (fc - fi)
    end
    fi = fi + 1
    Vis.bgcolor(Vis.flist, j, bgc, 0, 0)
end
Vis.bgcolor(Vis.flist, tonefadestop+Vis.frames2msec(1), 0, 0, 0)

end
