Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")

random = math.random
function rand3f(low, high)
    return random(low*1000, high*1000) / 100.0
end
function rand01()
    return random(0, 100) / 100.0
end

function do_emit_table(start)
    t = VisUtil.make_emit_table()
    t.count = 1000
    t.when = start
    VisUtil.center_emit_table(t, Vis.WIDTH/2, Vis.HEIGHT/2)
    t.ds = 2
    t.uds = 1
    t.utheta = math.pi
    t.life = 200
    t.ulife = random(0, 20)
    VisUtil.color_emit_table(t, rand01(), rand01(), rand01(),
                                rand01(), rand01(), rand01())
    t.limit = Vis.LIMIT_SPRINGBOX
    VisUtil.emit_table(t)
end

do_emit_table(0)
Vis.mutate(Vis.flist, 50, Vis.MUTATE_SLOW, 0.1)
Vis.mutate(Vis.flist, 75, Vis.MUTATE_PUSH, 15.0)
do_emit_table(100)
Vis.mutate(Vis.flist, 125, Vis.MUTATE_PUSH_DX, 2)
Vis.mutate(Vis.flist, 150, Vis.MUTATE_PUSH_DY, 2)

Vis.command(Vis.flist, 200, "exit")
