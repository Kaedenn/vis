Vis = require("Vis")

Vis.audio("media/Bowser.wav")

function make_emit_table()
    t = {}
    t.count = 0
    t.when = 0
    t.x = 0
    t.y = 0
    t.ux = 0
    t.uy = 0
    t.radius = 0
    t.uradius = 0
    t.ds = 0
    t.uds = 0
    t.theta = 0
    t.utheta = 0
    t.life = 0
    t.ulife = 0
    t.r = 0
    t.g = 0
    t.b = 0
    t.ur = 0
    t.ug = 0
    t.ub = 0
    t.force = Vis.DEFAULT_FORCE
    t.limit = Vis.DEFAULT_LIMIT
    t.blender = Vis.BLEND_LINEAR
    return t
end

function emit_table(t)
    Vis.emit(Vis.flist, t.count, t.when, t.x, t.y, t.ux, t.uy, t.radius, t.uradius, t.ds, t.uds, t.theta, t.utheta, t.life, t.ulife, t.r, t.g, t.b, t.ur, t.ug, t.ub, t.force, t.limit, t.blender)
end

t = make_emit_table()
t.x = 400
t.y = 300
t.radius = 1
t.ds = 2
t.uds = 2
t.life = 100
t.ulife = 10
t.r = 0
t.g = 100
t.b = 200
t.ur = 0
t.ug = 10
t.ub = 50
for i = 0, 100 do
    t.count = 1000
    t.when = i
    t.theta = 6.28 * i / 100
    t.utheta = 6.28 / 100
    emit_table(t)
end

Vis.command(Vis.flist, 100 + 100 + 10, "exit")
