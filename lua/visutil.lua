Vis = require('Vis')
bit32 = require('bit32')

VisUtil = {}

VisUtil.EMIT_FIELDS = {
    "count", "when", "x", "y", "ux", "uy", "radius", "uradius",
    "ds", "uds", "theta", "utheta", "life", "ulife", "r", "g", "b",
    "ur", "ug", "ub", "force", "limit", "blender"
}

VisUtil.COLOR = {}
VisUtil.COLOR.BLUE1 = {0, 0.2, 0.8, 0, 0.1, 0.2}
VisUtil.COLOR.BLUE2 = {0, 0.2, 0.8, 0, 0.21, 0.21}
VisUtil.COLOR.GREEN1 = {0, 0.8, 0.2, 0, 0.2, 0.1}
VisUtil.COLOR.GREEN2 = {0, 0.8, 0.2, 0, 0.21, 0.11}

VisUtil.Debug = {}
VisUtil.Debug.ENABLED = (function()
    if os.getenv('VIS_DEBUG') ~= nil then return true end
    if os.getenv('LUA_DEBUG') ~= nil then return true end
    if os.getenv('DEBUG') ~= nil then return true end
    if Vis.DEBUG > Vis.DEBUG_NONE then return true end
    return false
end)()
setmetatable(VisUtil.Debug, {
    __tostring = function(self) return VisUtil.strobject(self) end,
    __call = function(self, level, str, ...)
        if VisUtil.Debug.ENABLED ~= true then return end

        local varargs = {...}
        if type(level) == "table" then
            table.insert(varargs, 1, str)
            str, level = VisUtil.strobject(level), 0
        elseif type(level) == "string" then
            table.insert(varargs, 1, str)
            str, level = level, 0
        elseif type(level) == "number" and type(str) ~= "string" then
            str = VisUtil.strobject(str)
        end

        local ar = debug.getinfo(2)
        local wrap = ('Debug: %s:%s:%d: %%s'):format(
            ar.name or '<top>', ar.short_src,
            ar.linedefined ~= 0 and ar.linedefined or ar.currentline)
        if bit32.band(level, VisUtil.Debug.L_INSPECT) > 0 then
            -- L_INSPECT implies L_ESCAPE
            wrap = ("Debug(%s, %s, %s)"):format(self, wrap, VisUtil.strobject(varargs))
        elseif bit32.band(level, VisUtil.Debug.L_ESCAPE) > 0 then
            wrap = ("Debug(%s, %s)"):format(VisUtil.strobject(self), wrap)
        end
        if bit32.band(level, VisUtil.Debug.L_TRACEBACK) > 0 then
            wrap = ("%s: %s"):format(wrap, debug.traceback())
        end

        if #varargs > 0 then
            str = str:format(table.unpack(varargs))
        end
        print(wrap:format(str))
    end
})

VisUtil.Debug.TRACEBACK = false

VisUtil.Debug.L_ESCAPE = 1
VisUtil.Debug.L_TRACEBACK = 2
VisUtil.Debug.L_INSPECT = 4

VisUtil.Debug.Print = function(msg, args)
    if args ~= nil then
        msg = msg:format(args)
    end
    if VisUtil.Debug.ENABLED == true then
        print(("Debug: %s"):format(msg))
    end
    if VisUtil.Debug.TRACEBACK == true then
        print(debug.traceback())
    end
end

VisUtil.Debug.QUOTE_CHARS = {}
VisUtil.Debug.QUOTE_CHARS["\a"] = "\\a"
VisUtil.Debug.QUOTE_CHARS["\b"] = "\\b"
VisUtil.Debug.QUOTE_CHARS["\f"] = "\\f"
VisUtil.Debug.QUOTE_CHARS["\n"] = "\\n"
VisUtil.Debug.QUOTE_CHARS["\r"] = "\\r"
VisUtil.Debug.QUOTE_CHARS["\t"] = "\\t"
VisUtil.Debug.QUOTE_CHARS["\v"] = "\\v"
VisUtil.Debug.QUOTE_CHARS["\\"] = "\\\\"
VisUtil.Debug.QUOTE_CHARS['"'] = '\\"'

VisUtil.Debug.QuoteString = function(str)
    local result = ''
    for i = 1,#str do
        b = str:byte(i)
        c = string.char(b)
        if VisUtil.Debug.QUOTE_CHARS[c] ~= nil then
            result = result .. VisUtil.Debug.QUOTE_CHARS[c]
        elseif 0x20 <= b and b < 0x7F then
            result = result .. c
        else
            result = result .. ("\\x%02x"):format(b)
        end
    end
    return '"' .. result .. '"'
end

VisUtil.Args = {
    ERRORS = {
        E_NILVAL = "nil passed where a %s is expected",
        E_NOTANUM = "%s passed where a number is expected",
        E_NOTABOOL = "%s passed where a boolean is expected",
        E_UNKVALTP = "unknown value type %s"
    }
}
function VisUtil.Args:new()
    local o = {}
    o._args = {}
    o._environ = {}
    o._arg_env_link = {}
    o._getenv = os.getenv
    setmetatable(o, self)
    self.__index = self
    return o
end
function VisUtil.Args:add(arg, valuetype)
    self._args[arg] = valuetype
end
function VisUtil.Args:add_env(arg, valuetype)
    self._environ[arg] = valuetype
end
function VisUtil.Args:link_arg_env(arg, env)
    self._arg_env_link[arg] = env
end
function VisUtil.Args:parse(args)
    local errtab = VisUtil.Args.ERRORS
    if args == nil then args = Arguments end
    local parsed = {}
    local parsedenv = {}
    local function doerror(errstr, arg) return nil, errstr:format(arg) end
    local function ensure(value, valuetype)
        VisUtil.Debug(("ensure(%s, %s)"):format(tostring(value), tostring(valuetype)))
        if valuetype == nil or valuetype == "nil" then
            return 1
        elseif value == nil then
            return doerror(errtab.E_NILVAL, valuetype)
        elseif valuetype == "string" then
            return value
        elseif valuetype == "number" then
            if type(value) ~= "string" or value:match("^[0-9]*$") == nil then
                return doerror(errtab.E_NOTANUM, value)
            end
            return tonumber(value)
        elseif valuetype == "boolean" or valuetype == "bool" then
            local strbool = {"false", "true"}
            if value == "true" then return true end
            if value == "false" then return false end
            if value == "1" then return true end
            if value == "0" then return false end
            return doerror(errtab.E_NOTABOOL, value)
        else
            return doerror(errtab.E_UNKVALTP, valuetype)
        end
    end
    for idx = 1,#args do
        for arg,val in pairs(self._args) do
            if args[idx] == arg then
                if val == nil then
                    parsed[arg] = 1
                else
                    local ensured, err = ensure(args[idx+1], val)
                    if ensured == nil then
                        return nil, nil, err
                    end
                    parsed[arg] = ensured
                    idx = idx + 1
                end
                break
            end
        end
    end
    for env,envtype in pairs(self._environ) do
        if self._getenv(env) ~= nil then
            parsedenv[env] = ensure(self._getenv(env), envtype)
        end
    end
    for arg,env in pairs(self._arg_env_link) do
        if parsed[arg] ~= parsedenv[env] then
            if parsed[arg] ~= nil and parsedenv[env] ~= nil then
                parsed[arg] = parsedenv[env]
            elseif parsed[arg] == nil and parsedenv[env] == nil then
                -- wat
            elseif parsed[arg] == nil then
                parsed[arg] = parsedenv[env]
            elseif parsedenv[env] == nil then
                parsedenv[env] = parsed[arg]
            end
        end
    end
    return parsed, parsedenv, nil
end

function VisUtil.wrap_coord(x, y)
    while x < 0 do x = Vis.WIDTH + x end
    while y < 0 do y = Vis.HEIGHT + y end
    while x > Vis.WIDTH do x = x - Vis.WIDTH end
    while y > Vis.HEIGHT do y = y - Vis.HEIGHT end
    return x, y
end

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
    t.r = r and r>1 and r/255.0 or (r or 0)
    t.g = g and g>1 and g/255.0 or (g or 0)
    t.b = b and b>1 and b/255.0 or (b or 0)
    t.ur = ur and ur>1 and ur/255.0 or (ur or 0)
    t.ug = ug and ug>1 and ug/255.0 or (ug or 0)
    t.ub = ub and ub>1 and ub/255.0 or (ub or 0)
end

function VisUtil.color_emit_table_v(t, rgb)
    VisUtil.color_emit_table(t, rgb[1], rgb[2], rgb[3],
                                rgb[4], rgb[5], rgb[6])
end

function VisUtil.color_emit_table_v2(t, rgb, urgb)
    local urgb = urgb or {0, 0, 0}
    VisUtil.color_emit_table(t, rgb[1], rgb[2], rgb[3],
                                urgb[1], urgb[2], urgb[3])
end

function VisUtil.emit_table(t)
    local x
    local y
    x, y = VisUtil.wrap_coord(t.x, t.y)
    Vis.emit(Vis.flist, t.count, t.when, x, y, t.ux, t.uy,
             t.radius, t.uradius, t.ds, t.uds, t.theta, t.utheta,
             t.life, t.ulife, t.r, t.g, t.b, t.ur, t.ug, t.ub,
             t.force, t.limit, t.blender)
end

function VisUtil.emit_table_now(t)
    local x
    local y
    x, y = VisUtil.wrap_coord(t.x, t.y)
    Vis.emitnow(Vis.script, t.count, x, y, t.ux, t.uy,
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

function VisUtil.mutate_tag(when, tagop, tagval)
    Vis.mutate(Vis.flist, when, tagop, tagval)
end

function VisUtil.mutate_if(when, method, factor, methodcond, tagval)
    Vis.mutate(Vis.flist, when, method, factor, methodcond, tagval)
end

function VisUtil.stremit(e)
    local s = ""
    for _,v in pairs(VisUtil.EMIT_FIELDS) do
        s = ("%s\t%s: %s,\n"):format(s, v, e:get(v))
    end
    return ("e = {\n%s}"):format(s)
end

function VisUtil.strobject(o, i, seen, whereami, maxi)
    local i = i or 0
    local seen = seen or {}
    local whereami = whereami or {"_G"}
    local maxi = maxi or math.huge
    local function indent(n) return string.rep(" ",n) end
    if type(o) == "string" then
        return VisUtil.Debug.QuoteString(o)
    elseif type(o) == "number" then
        return tostring(o)
    elseif type(o) == "function" then
        if seen[o] == nil then
            seen[o] = table.concat(whereami, '.')
            local ar = debug.getinfo(o)
            if ar.short_src == '[C]' then
                return tostring(o)
            end
            return VisUtil.strobject(ar, i, seen, whereami)
        else
            return seen[o]
        end
    elseif o == nil then
        return "nil"
    elseif type(o) == "boolean" then
        return tostring(o)
    elseif type(o) ~= "table" then
        return ("%s --[[ %s --]]"):format(tostring(o), type(o))
    elseif i >= maxi then
        return "{ --[[ max depth exceeded --]] }"
    elseif seen[o] == nil then
        seen[o] = table.concat(whereami, ".")
        if next(o) == nil then
            return "{ } --[[ empty --]]"
        end
        local s = "{"
        local f = true
        local sorted = {}
        for k,v in pairs(o) do
            table.insert(sorted, k)
        end
        table.sort(sorted)
        for _,k in pairs(sorted) do
            local v = o[k]
            local k_str = ''
            if f then
                f = false
            else
                s = s..","
            end
            -- An interesting idea, saved for later
            --s = s.."\n"..table.concat(whereami, '.')
            s = s.."\n"..indent(i+2)
            if type(k) == "string" then
                k_str = ("[%s]"):format(VisUtil.Debug.QuoteString(k))
                whereami[#whereami+1] = tostring(k)
            elseif type(k) == "number" then
                k_str = ("[%d]"):format(k)
                whereami[#whereami+1] = k_str
            else
                k_str = ("[%s]"):format(VisUtil.strobject(k, 0, seen, whereami, maxi))
                whereami[#whereami+1] = k_str
            end
            s = s..k_str.." = "..VisUtil.strobject(v, i+2, seen, whereami, maxi)
            whereami[#whereami] = nil
        end
        s = s.."\n"..indent(i).."}"
        return s
    else
        return seen[o]
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
