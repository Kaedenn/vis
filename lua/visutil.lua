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
    t.r = r>1 and r/255.0 or (r or 0)
    t.g = g>1 and g/255.0 or (g or 0)
    t.b = b>1 and b/255.0 or (b or 0)
    t.ur = ur>1 and ur/255.0 or (ur or 0)
    t.ug = ug>1 and ug/255.0 or (ug or 0)
    t.ub = ub>1 and ub/255.0 or (ub or 0)
end

function VisUtil.emit_table(t)
    Vis.emit(Vis.flist, t.count, t.when, t.x, t.y, t.ux, t.uy, t.radius, t.uradius, t.ds, t.uds, t.theta, t.utheta, t.life, t.ulife, t.r, t.g, t.b, t.ur, t.ug, t.ub, t.force, t.limit, t.blender)
end

function VisUtil.seek_to(t)
    Vis.seek(t / 10)
    Vis.seekms(Vis.flist, 0, t)
end

function VisUtil.set_trace_table(t)
    Vis.settrace(Vis.script, t.count, t.x, t.y, t.ux, t.uy, t.rad, t.urad, t.ds, t.uds, t.theta, t.utheta, t.life, t.ulife, t.r, t.g, t.b, t.ur, t.ug, t.ub, t.force, t.limit, t.blender)
end

return VisUtil
