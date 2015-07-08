assert(require('Vis'))
assert(require('visutil'))

Vis = require("Vis")
VisUtil = require("visutil")

function foo() end

Vis.debug("Launching 1_simple.lua", foo)

t = VisUtil.make_emit_table()
VisUtil.center_emit_table(t, 400, 300)
t.radius = 1
t.ds = 2
t.uds = 0.75
t.life = 1000
VisUtil.color_emit_table(t, 0, 100, 200, 0, 50, 100)
t.count = 1000
t.theta = math.pi
t.utheta = math.pi
VisUtil.emit_table(t)

Vis.exit(Vis.flist, 1000)
