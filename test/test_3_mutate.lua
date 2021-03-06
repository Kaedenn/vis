Harness = require('harness')
Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")

Vis.on_quit(function()
    assert(Vis.get_debug(Vis.script, "NUM-MUTATES") == 4,
           "performed four mutates")
end)

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
    t.life = Vis.frames2msec(51)
    t.ulife = Vis.frames2msec(random(0, 20))
    VisUtil.color_emit_table(t, rand01(), rand01(), rand01(),
                                rand01(), rand01(), rand01())
    t.limit = Vis.LIMIT_SPRINGBOX
    VisUtil.emit_table(t)
end

do_emit_table(0)
Vis.mutate(Vis.flist, Vis.frames2msec(10), Vis.MUTATE_SLOW, 0.1)
Vis.mutate(Vis.flist, Vis.frames2msec(20), Vis.MUTATE_PUSH, 15.0)
do_emit_table(Vis.frames2msec(30))
Vis.mutate(Vis.flist, Vis.frames2msec(30), Vis.MUTATE_PUSH_DX, 2)
Vis.mutate(Vis.flist, Vis.frames2msec(40), Vis.MUTATE_PUSH_DY, 2)

Vis.exit(Vis.flist, Vis.frames2msec(60))
