
VisUtil = require('visutil')

Emit = {}

local function _deduce_tag_id(obj)
    if obj then
        if obj.tag then
            return obj.tag
        end
        return tostring(obj)
    end
    return math.random()
end

function Emit:new(obj)
    local tagid = _deduce_tag_id(obj)
    local o = {
        _t = VisUtil.make_emit_table(),
        _tagid = tagid,
        _tag = ("Emit(%s)"):format(tagid)
    }
    o._t = {}
    o._t = VisUtil.make_emit_table()
    o._t.tag = tagid
    if obj ~= nil and obj._t ~= nil then
        for k,v in pairs(obj._t) do
            o._t[k] = v
        end
    end
    setmetatable(o, self)
    self.__index = self
    return o
end

function Emit:copy() return Emit:new(self) end
function Emit:clone() return self:copy() end
function Emit:emit() VisUtil.emit_table(self._t) end
function Emit:emit_at(t) self:when(t); return self:emit() end
function Emit:emit_now() return VisUtil.emit_table_now(self._t) end
function Emit:set_trace() return VisUtil.set_trace_table(self._t) end
function Emit:str() return VisUtil.stremit(self) end
function Emit:get(k) return self._t[k] end
function Emit:set(k,v) self._t[k] = v end
function Emit:update(t)
    for k, v in pairs(t) do
        self:set(k, v)
    end
end

function Emit:count(n)
    self._t.count = n
end

function Emit:when(t)
    self._t.when = t
end

function Emit:center(x, y, ux, uy, s, us)
    self._t.s = s or 0
    self._t.us = us or 0
    return VisUtil.center_emit_table(self._t, x, y, ux or 0, uy or 0)
end

function Emit:move(dx, dy, udx, udy)
    self._t.x = self._t.x + dx
    self._t.y = self._t.y + dy
    self._t.ux = self._t.ux + udx or 0
    self._t.uy = self._t.uy + udy or 0
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

function Emit:_parse_color(r, g, b, ur, ug, ub)
    local rgb, urgb
    if type(r) == "table" then
        rgb = {r[1] or 1, r[2] or 1, r[3] or 1}
        if type(g) == "table" then
            urgb = {g[1] or 0, g[2] or 0, g[3] or 0}
        elseif #r > 3 then
            urgb = {r[4] or 0, r[5] or 0, r[6] or 0}
        else
            urgb = {0, 0, 0}
        end
    else
        rgb = {r or 1, g or 1, b or 1}
        urgb = {ur or 0, ug or 0, ub or 0}
    end
    assert(rgb[1] ~= nil and rgb[2] ~= nil and rgb[3] ~= nil)
    assert(urgb[1] ~= nil and urgb[2] ~= nil and urgb[3] ~= nil)
    return rgb, urgb
end

function Emit:color(r, g, b, ur, ug, ub)
    local rgb, urgb = self:_parse_color(r, g, b, ur, ug, ub)
    self._t.r = rgb[1]
    self._t.g = rgb[2]
    self._t.b = rgb[3]
    self._t.ur = urgb[1]
    self._t.ug = urgb[2]
    self._t.ub = urgb[3]
end

function Emit:fadeto(r, g, b, ur, ug, ub)
    local rgb, urgb = self:_parse_color(r, g, b, ur, ug, ub)
    self._t.r = self._t.r + r
    self._t.g = self._t.g + g
    self._t.b = self._t.b + b
    self._t.ur = self._t.ur + ur
    self._t.ug = self._t.ug + ug
    self._t.ub = self._t.ub + ub
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

function Emit:tag(tag)
    self._t.tag = tag
end

return Emit

