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

    -- Similar notes: p9 ~ p14: high med low HIGH MED LOW pitch
    34234, 34341, -- m8p9  3
    34356, 34466, -- m8p10 4
    34482, 34591, -- m8p11 5
    34604, 34717, -- m8p12 7
    34735, 34810, -- m8p13 8
    34858, 34934, -- m8p14 9

    -- Similar short notes: p15 ~ p26
    34983, 35061, -- m8p15 10 (pitch guide)
    35108, 35186, -- m8p16  9
    35239, 35292, -- m8p17  8
    35358, 35417, -- m8p18  7
    35484, 35539, -- m8p19  2
    35609, 35667, -- m8p20  3
    35735, 35790, -- m8p21  4
    35858, 35914, -- m8p22  5
    35985, 36040, -- m8p23  6
    36108, 36167, -- m8p24  7
    36235, 36293, -- m8p25  8
    36359, 36478, -- m8p26  9
    
    -- Longer notes: p27 ~ p29
    36608, 36729, -- m8p27 high
    36859, 36978, -- m8p28 higher
    37109, 37292, -- m8p29 higher+

    -- Long tones: p30 ~ p36
    37359, 37777, -- m8p30.a higher++
    37777, 38219, -- m8p30.b higher+++
    38235, 38593, -- m8p31 same
    38608, 38969, -- m8p32 same
    38987, 39220, -- m8p33 same
    39240, 39598, -- m8p34 same
    39610, 39972, -- m8p35 higher++++
    39988, 40723, -- m8p36 higher+++

    -- Similar short notes: p37 ~
    40738, 40848, -- m8p37
}

local e = Emit:new()
e:count(100)
e:radius(2)
e:ds(3, 0.5)
e:theta(math.pi, math.pi)
e:life(SECOND/4)
e:color({0.9, 0.9, 0.9, 0.09, 0.09, 0.09})

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
-- There's too much visual stimulation, so skip these next three notes.
-- Basically, I'm unable to descern these three notes apart from the rest of
-- the visualization, and I can't get anything good out of them. So, skip.
for i = 1,6 do T8.next() end -- skip m8p1.2{5,6,7}

-- The tone
local tonestart = T8.next() -- m8p2
local tonestop = T8.next() -- m8p2
local tonefadestart = T8.next() -- m8p2-fade
local tonefadestop = T8.next() -- m8p2-fade
local bgc = 0
local fc = math.floor((tonefadestop - tonestart)/Vis.frames2msec(1))
local fi = 0
for j = tonestart,tonefadestop,Vis.frames2msec(1) do
    if fi <= 5 then
        bgc = 0.05 * 2*fi
        Vis.bgcolor(Vis.flist, j, bgc, 0, 0)
    elseif fi >= fc-10 then
        bgc = 0.05 * (fc - fi)
        Vis.bgcolor(Vis.flist, j, bgc, 0, 0)
    end
    fi = fi + 1
end

for rep = 1,2 do
    local pos = {
        {W(1,5), H(4,5)},
        {-W(1,5), H(4,5)}
    }
    for i = 1,2 do
        start = T8.next()
        stop = T8.next()
        e:center(pos[i][1], pos[i][2])
        for j = start,stop,Vis.frames2msec(1) do
            e:emit_at(j)
        end
    end

    tonestart = T8.next()
    tonestop = T8.next()
    bgc = 0
    fc = (tonestop - tonestart)/Vis.frames2msec(1)
    fi = 0
    for j = tonestart,tonestop,Vis.frames2msec(1) do
        if fi <= 5 then
            bgc = bgc + 0.10
            Vis.bgcolor(Vis.flist, j, bgc, 0, 0)
        elseif fi > fc-5 then
            bgc = bgc - 0.10
            Vis.bgcolor(Vis.flist, j, bgc, 0, 0)
        end
        fi = fi + 1
    end
end

end
