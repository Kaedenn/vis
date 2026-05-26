Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")

SECOND = 1000
Vis.exit(Vis.flist, 10*SECOND)

t = VisUtil.make_emit_table()
t.count = 100
t.ds = 5
t.uds = 1
t.radius = 1
VisUtil.center_emit_table(t, Vis.WIDTH/2, Vis.HEIGHT/2)
VisUtil.color_emit_table(t, 0, 100, 200, 0, 50, 100)
VisUtil.color_emit_table(t, 0, .6, .9, 0, .4, .1)
t.life = 1000

dtheta = math.pi/SECOND*20
t.utheta = dtheta*3
for i = 0,4*SECOND,Vis.frames2msec(1) do
    if i < SECOND then
        t.ds = 4*i/SECOND
    else
        t.ds = 4
    end
    t.when = i
    t.theta = dtheta*i
    VisUtil.emit_table(t)
end

t.utheta = math.pi/50
t.ds = 5

ang = 0
dir = 1
for ctr = 0,10*SECOND,Vis.frames2msec(1) do
    t.when = ctr
    ang = ang + dir * t.utheta
    if dir == 1 and ang > math.pi then
        dir = -1
    elseif dir == -1 and ang < 0 then
        dir = 1
    end
    
    VisUtil.center_emit_table(t, Vis.WIDTH/2, 0)
    t.theta = ang
    VisUtil.emit_table(t)

    VisUtil.center_emit_table(t, Vis.WIDTH/2, Vis.HEIGHT)
    t.theta = ang + math.pi
    VisUtil.emit_table(t)

    VisUtil.center_emit_table(t, 0, Vis.HEIGHT/2)
    t.theta = ang + 3 * math.pi / 2
    VisUtil.emit_table(t)

    VisUtil.center_emit_table(t, Vis.WIDTH, Vis.HEIGHT/2)
    t.theta = ang + math.pi / 2
    VisUtil.emit_table(t)
end

