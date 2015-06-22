Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")

Vis.audio("media/Bowser.wav")

function emit_spiral(steps)
    local MAXLIFE = 100
    local MAXLIFE_ADJ = 10

    t = VisUtil.make_emit_table()
    center_emit_table(t)
    t.ds = 2
    t.uds = 1.75
    t.life = MAXLIFE
    t.ulife = MAXLIFE_ADJ
    VisUtil.color_emit_table(t, 0, 100, 200, 0, 10, 50)
    for i = 0, steps do
        t.count = 1000
        t.when = i
        t.theta = 2*math.pi * i / steps
        t.utheta = 2*math.pi / steps
        VisUtil.emit_table(t)
    end
    return steps + MAXLIFE + MAXLIFE_ADJ
end

function emit_circle(start, x, y, r, g, b, ur, ug, ub)
    local MAXLIFE = 10
    local MAXLIFE_ADJ = 0

    t = VisUtil.make_emit_table()
    VisUtil.center_emit_table(t, x or Vis.WIDTH/3, y or Vis.HEIGHT*2/3)
    t.ds = 1
    t.life = 10
    VisUtil.color_emit_table(t, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0);
    t.count = 100
    t.when = start
    t.theta = math.pi
    t.utheta = math.pi
    VisUtil.emit_table(t)
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

-- 200, 500 = Vis.WIDTH / 4, Vis.HEIGHT * 5 / 6
emit_circle(37, W_1_4, H_5_6)
emit_circle(46, W_3_4, H_5_6)
emit_circle(55, W_1_2, H_3_4)
j = 64
while j < 100 do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 1
end
emit_circle(104, W_1_2, H_2_3)
emit_circle(112, W_1_2, H_3_4)

emit_circle(126, W_1_2, H_2_3)
emit_circle(134, W_1_2, H_3_4)

emit_circle(148, W_1_2, H_2_3)
emit_circle(156, W_1_2, H_3_4)

Vis.seek(math.floor(215.0 / Vis.FPS_LIMIT) * 100)
Vis.seekframe(Vis.flist, 0, 215)

emit_circle(215, W_1_4, H_5_6)
emit_circle(226, W_3_4, H_5_6)
emit_circle(237, W_1_2, H_3_4)
j = 245
while j < 276 do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 1
end
emit_circle(284, W_1_4, H_5_6)
emit_circle(293, W_3_4, H_5_6)

--now = emit_spiral(100)
Vis.command(Vis.flist, 400, "exit")
