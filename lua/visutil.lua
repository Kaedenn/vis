Vis = require('Vis')

VisUtil = {}

VisUtil.EMIT_FIELDS = {
    "count", "when", "x", "y", "ux", "uy", "radius", "uradius",
    "ds", "uds", "theta", "utheta", "life", "ulife", "r", "g", "b",
    "ur", "ug", "ub", "force", "limit", "blender"
}

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

-- Performs a shallow copy, since emit tables are all shallow
function VisUtil.copy_table(t)
    local t2 = {}
    for k,v in pairs(t) do
        t2[k] = v
    end
    return t2
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
    t.ur = ur and ur>1 and ur/255.0 or (ur or 0)
    t.ug = ug and ug>1 and ug/255.0 or (ug or 0)
    t.ub = ub and ub>1 and ub/255.0 or (ub or 0)
end

function VisUtil.color_emit_table_v(t, rgb)
    VisUtil.color_emit_table(t, rgb[1], rgb[2], rgb[3],
                                rgb[4], rgb[5], rgb[6])
end

function VisUtil.color_emit_table_v2(t, rgb, urgb)
    urgb = urgb or {0, 0, 0}
    VisUtil.color_emit_table(t, rgb[1], rgb[2], rgb[3],
                                urgb[1], urgb[2], urgb[3])
end

function VisUtil.emit_table(t)
    Vis.emit(Vis.flist, t.count, t.when, t.x, t.y, t.ux, t.uy,
             t.radius, t.uradius, t.ds, t.uds, t.theta, t.utheta,
             t.life, t.ulife, t.r, t.g, t.b, t.ur, t.ug, t.ub,
             t.force, t.limit, t.blender)
end

function VisUtil.emit_table_now(t)
    Vis.emitnow(Vis.script, t.count, t.x, t.y, t.ux, t.uy,
                t.radius, t.uradius, t.ds, t.uds, t.theta, t.utheta,
                t.life, t.ulife, t.r, t.g, t.b, t.ur, t.ug, t.ub,
                t.force, t.limit, t.blender)
end

function VisUtil.seek_to(t)
    Vis.seek(t / 10)
    Vis.seekms(Vis.flist, 0, t)
end

function VisUtil.set_trace_table(t)
    Vis.settrace(Vis.script, t.count, t.x, t.y, t.ux, t.uy,
                 t.rad, t.urad, t.ds, t.uds, t.theta, t.utheta,
                 t.life, t.ulife, t.r, t.g, t.b, t.ur, t.ug, t.ub,
                 t.force, t.limit, t.blender)
end

function VisUtil.stremit(e)
    s = "e = {\n"
    for _,v in pairs(VisUtil.EMIT_FIELDS) do
        s = s .. "\t"..v..": "..e._t[v]..",\n"
    end
    s = s .. "}"
    return s
end

function VisUtil.strobject(o, i, seen)
    seen = seen or {}
    if type(o) ~= "table" then
        if type(o) == "string" then
            return string.format("%q", tostring(o))
        else
            return tostring(o)
        end
    elseif seen[o] == nil then
        seen[o] = 1
        local function indent(n)
            return string.rep(" ",n)
        end
        if next(o) == nil then
            return "{}"
        end
        i = i or 0
        local s = "{"
        local f = true
        for k,v in pairs(o) do
            if f then
                f = false
            else
                s = s..","
            end
            s = s.."\n"..indent(i+2)
            if type(k) == "string" then
                s = s .. string.format("%q", tostring(k))
            else
                s = s.."["..VisUtil.dumpobject(k, 0, seen).."]"
            end
            s = s.." = "..VisUtil.dumpobject(v, i+2, seen)
        end
        s = s.."\n"..indent(i).."}"
        return s
    else
        return "{...}"
    end
end

function VisUtil.genlua_force(force)
    if force == Vis.DEFAULT_FORCE then return "Vis.DEFAULT_FORCE" end
    if force == Vis.FORCE_FRICTION then return "Vis.FORCE_FRICTION" end
    if force == Vis.FORCE_GRAVITY then return "Vis.FORCE_GRAVITY" end
    if force == Vis.NFORCES then return "Vis.NFORCES" end
    return "<invalid-force>"
end

function VisUtil.genlua_limit(limit)
    if limit == Vis.DEFAULT_LIMIT then return "Vis.DEFAULT_LIMIT" end
    if limit == Vis.LIMIT_BOX then return "Vis.LIMIT_BOX" end
    if limit == Vis.LIMIT_SPRINGBOX then return "Vis.LIMIT_SPRINGBOX" end
    if limit == Vis.NLIMITS then return "Vis.NLIMITS" end
    return "<invalid-limit>"
end

function VisUtil.genlua_blender(blender)
    -- Vis.DEFAULT_BLEND == Vis.BLEND_LINEAR
    --if blender == Vis.DEFAULT_BLEND then return "Vis.DEFAULT_BLEND" end
    if blender == Vis.BLEND_NONE then return "Vis.BLEND_NONE" end
    if blender == Vis.BLEND_LINEAR then return "Vis.BLEND_LINEAR" end
    if blender == Vis.BLEND_QUADRATIC then return "Vis.BLEND_QUADRATIC" end
    if blender == Vis.BLEND_NEGGAMA then return "Vis.BLEND_NEGGAMA" end
    if blender == Vis.NBLENDS then return "Vis.NBLENDS" end
    return "<invalid-blend>"
end

return VisUtil
