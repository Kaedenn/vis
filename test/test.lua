Vis = require("Vis")

Vis.audio("media/Bowser.wav")

function make_emit_table()
    return {
        count = 0, when = 0,
        x = 0, y = 0, ux = 0, uy = 0,
        radius = 0, uradius = 0,
        ds = 0, uds = 0, theta = 0, utheta = 0,
        life = 0, ulife = 0,
        r = 0, g = 0, b = 0, ur = 0, ug = 0, ub = 0,
        force = Vis.DEFAULT_FORCE, limit = Vis.DEFAULT_LIMIT,
        blender = Vis.BLEND_LINEAR
    }
end

function center_emit_table(t, x, y, ux, uy)
    t.x = x
    t.y = y
    t.ux = ux or 0
    t.uy = uy or 0
end

function color_emit_table(t, r, g, b, ur, ug, ub)
    t.r = r
    t.g = g
    t.b = b
    t.ur = ur or 0
    t.ug = ug or 0
    t.ub = ub or 0
end

function emit_table(t)
    Vis.emit(Vis.flist, t.count, t.when, t.x, t.y, t.ux, t.uy, t.radius, t.uradius, t.ds, t.uds, t.theta, t.utheta, t.life, t.ulife, t.r, t.g, t.b, t.ur, t.ug, t.ub, t.force, t.limit, t.blender)
end

function emit_spiral(steps)
    local MAXLIFE = 100
    local MAXLIFE_ADJ = 10

    t = make_emit_table()
    center_emit_table(t, 400, 300)
    t.radius = 1
    t.ds = 2
    t.uds = 2
    t.life = MAXLIFE
    t.ulife = MAXLIFE_ADJ
    color_emit_table(t, 0, 100, 200, 0, 10, 50)
    for i = 0, steps do
        t.count = 1000
        t.when = i
        t.theta = 6.28 * i / steps
        t.utheta = 6.28 / steps
        emit_table(t)
    end
    return steps + MAXLIFE + MAXLIFE_ADJ
end

now = emit_spiral(100)
Vis.command(Vis.flist, now, "exit")
