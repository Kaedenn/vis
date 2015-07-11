
VisUtil = require('visutil')

Emit = {}

function Emit:new(obj)
    o = {}
    o._t = {}
    if obj ~= nil and obj._t ~= nil then
        for k,v in pairs(obj._t) do
            o._t[k] = v
        end
    else
        o._t = VisUtil.make_emit_table()
    end
    setmetatable(o, self)
    self.__index = self
    return o
end

function Emit:copy() return Emit:new(self) end
function Emit:clone() return self:copy() end
function Emit:emit() return VisUtil.emit_table(self._t) end
function Emit:emit_now() return VisUtil.emit_table_now(self._t) end
function Emit:set_trace() return VisUtil.set_trace_table(self._t) end
function Emit:str() return VisUtil.stremit(self) end
function Emit:get(k) return self._t[k] end
function Emit:set(k,v) self._t[k] = v end

function Emit:count(n)
    self._t.count = n
end

function Emit:when(t)
    self._t.when = t
end

function Emit:center(x, y)
    return VisUtil.center_emit_table(self._t, x, y)
end

function Emit:radius(radius, uradius)
    self._t.radius = radius or 1
    self._t.uradius = uradius or self._t.uradius or 1
end

function Emit:ds(ds, uds)
    self._t.ds = ds or 0
    self._t.uds = uds or self._t.uds or 0
end

function Emit:theta(theta, utheta)
    self._t.theta = theta or 0
    self._t.utheta = utheta or self._t.utheta or 0
end

function Emit:life(life, ulife)
    self._t.life = life or 0
    self._t.ulife = ulife or self._t.ulife or 0
end

function Emit:color(r, g, b, ur, ug, ub)
    return VisUtil.color_emit_table(self._t, r, g, b, ur, ug, ub)
end

function Emit:color2(rgb)
    return VisUtil.color_emit_table_v(self._t, rgb)
end

function Emit:color3(rgb, urgb)
    return VisUtil.color_emit_table_v2(self._t, rgb, urgb)
end

function Emit:force(force)
    self._t.force = force
end

function Emit:limit(limit)
    self._t.limit = limit
end

function Emit:blender(blend)
    self._t.blender = blend
end

return Emit

