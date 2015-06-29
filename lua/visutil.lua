Vis = require('Vis')

VisUtil = {}
function VisUtil.make_emit_table()
    return {
        count = 0, when = 0,
        x = Vis.WIDTH / 2, y = Vis.HEIGHT / 2, ux = 0, uy = 0,
        radius = 1, uradius = 0,
        ds = 0, uds = 0, theta = 0, utheta = 0,
        life = 0, ulife = 0,
        r = 1, g = 1, b = 1, ur = 0, ug = 0, ub = 0,
        force = Vis.DEFAULT_FORCE, limit = Vis.DEFAULT_LIMIT,
        blender = Vis.BLEND_LINEAR
    }
end

function VisUtil.center_emit_table(t, x, y, ux, uy)
    t.x = x or Vis.WIDTH / 2
    t.y = y or Vis.HEIGHT / 2
    t.ux = ux or 0
    t.uy = uy or 0
end

function VisUtil.color_emit_table(t, r, g, b, ur, ug, ub)
    t.r = r
    t.g = g
    t.b = b
    t.ur = ur or 0
    t.ug = ug or 0
    t.ub = ub or 0
end

function VisUtil.emit_table(t)
    Vis.emit(Vis.flist, t.count, t.when, t.x, t.y, t.ux, t.uy, t.radius, t.uradius, t.ds, t.uds, t.theta, t.utheta, t.life, t.ulife, t.r, t.g, t.b, t.ur, t.ug, t.ub, t.force, t.limit, t.blender)
end

function VisUtil.seek_to(frame)
    Vis.seek(math.floor(frame / Vis.FPS_LIMIT * 100))
    Vis.seekframe(Vis.flist, 0, frame)
end

return VisUtil
