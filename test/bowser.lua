Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")

assert(Vis.MUTATE_AGE)

TRACK_1 = 1
TRACK_2 = 2
TRACK_3 = 3
TRACK_4 = 4
TRACK_5 = 5
TRACK_6 = 6
TRACK_7 = 7
TRACK_8 = 8
TrackTimes = {0, 0, 0, 0, 0, 0, 0, 0}

SECOND = 1000

--[[ For debugging
Vis.command(Vis.flist, t, "exit")
VisUtil.seek_to(t)
--]]

function trackstep(track, length)
    TrackTimes[track] = TrackTimes[track] + length
end

Vis.audio("media/Bowser.wav")

function asserteq(a, b)
    if a ~= b then
        print(a.." is not equal to "..b)
        assert(a == b)
    end
end

function emit_spiral(steps)
    local MAXLIFE = 10/3*SECOND
    local MAXLIFE_ADJ = 1/3*SECOND

    et = VisUtil.make_emit_table()
    center_emit_table(et)
    et.ds = 2
    et.uds = 1.75
    et.life = MAXLIFE
    et.ulife = MAXLIFE_ADJ
    VisUtil.color_emit_table(et, 0, 100, 200, 0, 10, 50)
    for i = 0, steps do
        et.count = 1000
        et.when = i
        et.theta = 2*math.pi * i / steps
        et.utheta = 2*math.pi / steps
        VisUtil.emit_table(et)
    end
    return steps + MAXLIFE + MAXLIFE_ADJ
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

function emit_line_v(start, x, r, g, b, ur, ug, ub)
    local MAXLIFE = SECOND/3
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, x, 0, 0, Vis.HEIGHT)
    et.ds = 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0)
    et.count = 200
    et.when = start
    et.theta = math.pi/2
    VisUtil.emit_table(et)
    et.theta = math.pi*3/2
    VisUtil.emit_table(et)
end

function emit_line_h(start, y, r, g, b, ur, ug, ub)
    local MAXLIFE = SECOND/3
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, 0, y, Vis.WIDTH, 0)
    et.ds = 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0)
    et.count = 200
    et.when = start
    et.theta = 0
    VisUtil.emit_table(et)
    et.theta = math.pi
    VisUtil.emit_table(et)
end

W_1_2 = Vis.WIDTH / 2 -- 400
W_1_3 = Vis.WIDTH / 3 -- 266.6...
W_2_3 = W_1_3 * 2     -- 533.3...
W_1_4 = Vis.WIDTH / 4 -- 200
W_3_4 = W_1_4 * 3     -- 600
W_1_6 = Vis.WIDTH / 6 -- 133.3...
W_5_6 = W_1_6 * 5     -- 666.6...
W_1_8 = Vis.WIDTH / 8 -- 100

H_1_2 = Vis.HEIGHT / 2 -- 300
H_1_3 = Vis.HEIGHT / 3 -- 200
H_2_3 = H_1_3 * 2      -- 400
H_1_4 = Vis.HEIGHT / 4 -- 150
H_3_4 = H_1_4 * 3      -- 450
H_1_6 = Vis.HEIGHT / 6 -- 100
H_5_6 = H_1_6 * 5      -- 500
H_1_8 = Vis.HEIGHT / 8 -- 75

-- INTRO: PART ONE: MAIN TRACK
t = 1233
emit_circle(t, W_1_4, H_5_6); t = t + 300
emit_circle(t, W_3_4, H_5_6); t = t + 300
emit_circle(t, W_1_2, H_3_4); t = t + 300
j = t
t = t + 1200
while j < t do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
t = t + 133

emit_circle(t, W_1_2, H_2_3); t = t + 266
emit_circle(t, W_1_2, H_3_4); t = t + 466

emit_circle(t, W_1_2, H_2_3)
--Vis.mutate(Vis.flist, t, Vis.MUTATE_OPACITY, 0.9)
t = t + 266
emit_circle(t, W_1_2, H_3_4)
--Vis.mutate(Vis.flist, t, Vis.MUTATE_OPACITY, 0.4)
t = t + 466

emit_circle(t, W_1_2, H_2_3); t = t + 266
emit_circle(t, W_1_2, H_3_4); t = t + 1466 + 500

-- INTRO: PART TWO: MAIN TRACK
emit_circle(t, W_1_4, H_5_6); t = t + 366
emit_circle(t, W_3_4, H_5_6); t = t + 366
emit_circle(t, W_1_2, H_3_4); t = t + 266
j = t
t = t + 1033
while j < t do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
t = t + 266
emit_circle(t, W_1_4, H_5_6); t = t + 233
emit_circle(t, W_3_4, H_5_6); t = t + 166

j = t
t = t + 333
while j < t do
    emit_line_v(j, W_1_2)
    j = j + 33
end
t = t + 400

j = t
t = t + 333
while j < t do
    emit_line_v(j, W_1_2)
    --Vis.mutate(Vis.flist, j, Vis.MUTATE_OPACITY, 0.9)
    j = j + 33
end
t = t + 400

j = t
t = t + 333
while j < t do
    emit_line_v(j, W_1_2)
    --Vis.mutate(Vis.flist, j, Vis.MUTATE_AGE, 0.4)
    j = j + 33
end
t = t + 400

-- INTRO: PART THREE: MAIN TRACK
emit_circle(t, W_1_4, H_5_6); t = t + 10
emit_circle(t, W_3_4, H_5_6); t = t + 10
emit_circle(t, W_1_2, H_3_4); t = t + 10
j = t
t = t + 36
while j < t do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 1
end
t = t + 4

emit_circle(t, W_1_2, H_2_3); t = t + 9
emit_circle(t, W_1_2, H_3_4); t = t + 12

emit_circle(t, W_1_2, H_2_3); t = t + 9
emit_circle(t, W_1_2, H_3_4); t = t + 12

emit_circle(t, W_1_2, H_2_3); t = t + 9
emit_circle(t, W_1_2, H_3_4); t = t + 9

-- MAIN SONG: MIX ONE
Vis.command(Vis.flist, t + 12, "exit")
