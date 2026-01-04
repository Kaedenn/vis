Vis = require("Vis")
bit32 = require('bit32')

VisUtil = {}

VisUtil.EMIT_FIELDS = {
    "count", "when", "x", "y", "ux", "uy", "s", "us",
    "radius", "uradius", "ds", "uds", "theta", "utheta",
    "depth", "life", "ulife", "r", "g", "b", "ur", "ug", "ub",
    "force", "limit", "blender", "tag"
}

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
    __call = function(self, obj, ...)
        if VisUtil.Debug.ENABLED ~= true then return end

        local varargs = {...}
        local str = VisUtil.strobject(obj)

        local ar = debug.getinfo(2)
        local wrap = ('Debug: %s:%s:%d: %%s'):format(
            ar.name or '<top>', ar.short_src,
            ar.linedefined ~= 0 and ar.linedefined or ar.currentline)
        if VisUtil.Debug.TRACEBACK ~= false then
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

function VisUtil.Debug.Print(msg, args)
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

function VisUtil.Debug.QuoteString(str)
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

--[[ Argument parsing
--
-- Please use the new API:
--  parser = VisUtil.Args:new()
--  parser:add_argument{...}
--  args, envs = parser:parse_args{...}
--
-- Old API exists for backwards compatibility, but will be removed.
--]]
VisUtil.Args = {
    ERRORS = {
        E_NILVAL = "nil passed where a %s is expected",
        E_NOTANUM = "%s passed where a number is expected",
        E_NOTABOOL = "%s passed where a boolean is expected",
        E_UNKVALTP = "unknown value type %s"
    }
}
function VisUtil.Args:new(config)
    local o = {}
    o._config = {}
    if type(config) == "table" then
        o._config = config
    end
    setmetatable(o, self)
    self.__index = self

    -- New API
    o._arg_defs = {}
    o:add_argument{"-h", "--help", help="print this message and exit", env="VIS_HELP"}

    -- Old API
    o._args = {}
    o._envs = {}
    o._links = {}
    o._arglist = {}
    o._envlist = {}
    o._linklist = {}
    o._arghelp = {}
    o._envhelp = {}
    o._helpstrs = {}
    o._defaults = {}
    o._getenv = os.getenv -- for testing only
    o:add("-h", "nil", "this message")
    o:add("--help", "nil", "this message")
    o:add_env("VIS_HELP", "nil", "this message")
    o:link_arg_env("-h", "VIS_HELP")
    o:link_arg_env("--help", "VIS_HELP")
    return o
end

function VisUtil.Args:_ensure_self()
    assert(type(self) == "table")
    assert(type(self._args) == "table", VisUtil.strobject(self))
end

--[[ Add an argument
--
-- Table must contain either one or two indexed values: short and long option
-- Table may contain any (or all) of the following keys:
--  env: string         Environment variable linked to this arg
--  envs: table         Table of environment variables to link to this arg
--  help: string        Help text if -h or --help given
--  metavar: string     Label to use for the argument's value, instead of type
--  argtype: string     Type of argument (see below)
--  default: any        Argument value if none given
--
-- argtype can be one of the following strings:
--  "flag"      Argument is a flag (no value); value will be true or false
--  "number"    Argument requires a (possibly floating-point) number
--  "string"    Argument requires a value of any length
-- If argtype is nil, then "flag" is assumed.
--
-- If multiple environment variables are set for a given argument, then only
-- the last one (first env, then envs) is used. Later environment variables
-- overwrite earlier ones.
--]]
function VisUtil.Args:add_argument(argdef)
    local arg = {}
    if #argdef == 0 then error("argdef missing required argument(s)") end
    if #argdef == 1 then
        if argdef[1]:match("^--") ~= "" then
            arg.short = nil
            arg.long = argdef[1]
        elseif argdef[1]:match("^-") ~= "" then
            arg.short = argdef[1]
            arg.long = nil
        else
            error(("argument %s missing hyphen"):format(argdef[1]))
        end
    else
        arg.short = argdef[1]
        arg.long = argdef[2]
    end
    arg.env = argdef.env
    arg.envs = argdef.envs or {}
    arg.help = argdef.help
    arg.argtype = argdef.argtype or "flag"
    arg.metavar = argdef.metavar or arg.argtype
    arg.default = argdef.default

    -- Ensure the argument doesn't already exist
    for _, existing in ipairs(self._arg_defs) do
        if arg.short == existing.short or arg.long == existing.long then
            error(("Argument %s duplicates existing argument"):format(argdef[1]))
        end
    end
    table.insert(self._arg_defs, arg)
end

--[[ Helper: print a debug message ]]
function VisUtil.Args:_debug(...)
    if os.getenv("VIS_ARG_DEBUG") then
        print("DEBUG: ", ...)
    end
end

--[[ Helper: does the given argument match the argument definition? ]]
function VisUtil.Args:_check_match(argdef, argval)
    if argdef.short and argval == argdef.short then return true end
    if argdef.long and argval == argdef.long then return true end
    return false
end

--[[ Actually parse the arguments; uses script args if args is nil ]]
function VisUtil.Args:parse_args(args)
    if not args then args = Arguments end
    if type(args) ~= "table" then
        error("Attempt to parse arguments that aren't a table")
    end

    -- Ensure the argument value matches the given type
    local function ensure(argval, argtype)
        self:_debug(("ensure(%s, %s)"):format(argval, argtype))
        if argtype == "flag" then return true, nil end
        if argtype == "number" then
            local ensured = tonumber(argval)
            if ensured == nil then
                return nil, ("argument %s not a number"):format(argval)
            end
            return ensured, nil
        end
        if argtype == "string" then return argval, nil end
        return nil, ("unknown argument type %s"):format(tostring(argtype))
    end

    local arg, env = {}, {}
    -- Pre-populate default values
    for _, argdef in ipairs(self._arg_defs) do
        local default_value = argdef.default
        local argenvs = {}
        if argdef.envs then argenvs = argdef.envs end
        if argdef.env then table.insert(argenvs, argdef.env) end
        for _, argenv in ipairs(argenvs) do
            local env_value = os.getenv(argenv)
            if env_value ~= nil then
                local ensured, err = ensure(env_value, argdef.argtype)
                if err ~= nil then
                    error(("Env %s requires value of type %s; got %s"):format(
                        argenv, argdef.argtype, env_value))
                end
                default_value = env_value
            end
        end
        if default_value ~= nil then
            if argdef.short then arg[argdef.short] = default_value end
            if argdef.long then arg[argdef.long] = default_value end
            for _, argenv in ipairs(argenvs) do
                env[argenv] = default_value
            end
        end
    end

    for argname, value in pairs(arg) do
        self:_debug(("arg %s has value %s"):format(argname, value))
    end
    for argname, value in pairs(env) do
        self:_debug(("env %s has value %s"):format(argname, value))
    end

    local idx = 1
    while idx <= #args do
        local matched = false
        local argv = args[idx]
        self:_debug(("Parsing argument %d: %s"):format(idx, argv))
        for _, argdef in ipairs(self._arg_defs) do
            if self:_check_match(argdef, argv) then
                matched = true
                local arg_value = true

                self:_debug(("%s matches argdef %s"):format(argv,
                    self:_arg_to_string(argdef)))

                if argdef.argtype ~= "flag" then
                    self:_debug(("%s requires value of type %s"):format(
                        argv, argdef.argtype))

                    local ensured, err = ensure(args[idx+1], argdef.argtype)
                    if err ~= nil then
                        error(("Argument %s requires value of type %s; got %s"):format(
                            argv, argdef.argtype, err))
                    end
                    arg_value = ensured
                    idx = idx + 1
                end
                if argdef.short then arg[argdef.short] = arg_value end
                if argdef.long then arg[argdef.long] = arg_value end
                if argdef.env then env[argdef.env] = arg_value end
                if argdef.envs then
                    for _, argenv in ipairs(argdef.envs) do
                        env[argenv] = arg_value
                    end
                end
                break
            end
        end
        if not matched then
            error(("Unknown argument %s"):format(args[idx]))
        end
        idx = idx + 1
    end

    if arg["-h"] then
        self:print_help(Vis.SCRIPT_NAME)
        os.exit()
    end

    return arg, env
end

--[[ Helper: convert the arg's short and long values to a single string ]]
function VisUtil.Args:_arg_to_string(argdef)
    local arg_str = argdef.short or argdef.long
    if argdef.short and argdef.long then
        arg_str = ("%s, %s"):format(argdef.short, argdef.long)
    end
    if argdef.argtype and argdef.argtype ~= "flag" then
        arg_str = arg_str .. " [" .. argdef.metavar .. "]"
    end
    return arg_str
end

--[[ Helper: convert an arg to a string (for help display) ]]
function VisUtil.Args:_arg_format_help(argdef, maxwidth)
    local result = self:_arg_to_string(argdef)
    if maxwidth ~= nil then
        result = result .. (" "):rep(maxwidth - #result)
    end
    result = result .. "  "
    if argdef.help then
        result = result .. " " .. argdef.help
    end
    if argdef.env then
        result = result .. (" (env %s)"):format(argdef.env)
    end
    if argdef.default then
        result = result .. (" (default: %s)"):format(tostring(argdef.default))
    end
    return result
end

--[[ Format the help text for all arguments ]]
function VisUtil.Args:format_help(execname)
    local arg_maxlen = 0
    for _, entry in ipairs(self._arg_defs) do
        local arg_str = self:_arg_to_string(entry)
        arg_maxlen = math.max(arg_maxlen, #arg_str)
    end

    local arg_lines = {
        ("Usage: %s [arguments]"):format(execname or "<program>"),
        ""
    }
    if self._config.prolog then
        for line in self._config.prolog:gmatch("[^\n]*[\n]?") do
            local line_clean = line:gsub("\n", "")
            if line_clean ~= "" then
                table.insert(arg_lines, line_clean)
            end
        end
        table.insert(arg_lines, "")
    end

    table.insert(arg_lines, "Arguments:")
    for _, entry in pairs(self._arg_defs) do
        local arg_str = self:_arg_format_help(entry, arg_maxlen)
        table.insert(arg_lines, "    " .. arg_str)
    end

    if self._config.epilog then
        table.insert(arg_lines, "")
        for line in self._config.epilog:gmatch("[^\n]*[\n]?") do
            local line_clean = line:gsub("\n", "")
            if line_clean ~= "" then
                table.insert(arg_lines, line_clean)
            end
        end
    end

    return table.concat(arg_lines, "\n")
end

--[[ Print the help text for all arguments ]]
function VisUtil.Args:print_help(execname)
    print(self:format_help(execname))
end

--[[ OLD API ]]

function VisUtil.Args:add(arg, valuetype, help)
    self:_ensure_self()
    self._args[arg] = valuetype
    self._arghelp[arg] = help
    table.insert(self._arglist, {[arg] = valuetype})
end

function VisUtil.Args:add_env(arg, valuetype, help)
    self:_ensure_self()
    self._envs[arg] = valuetype
    self._envhelp[arg] = help
    table.insert(self._envlist, {[arg] = valuetype})
end

function VisUtil.Args:link_arg_env(arg, env)
    self:_ensure_self()
    self._links[arg] = env
    table.insert(self._linklist, {[arg] = env})
end

function VisUtil.Args:set_default(arg, value)
    self:_ensure_self()
    self._defaults[arg] = value
end

function VisUtil.Args:add_help(helpstr)
    self:_ensure_self()
    self._helpstrs[#self._helpstrs+1] = helpstr
end

function VisUtil.Args:_get_max_arglen(args)
    self:_ensure_self()
    local maxw = 0
    for arg, argtype in pairs(args or self._args) do
        maxw = math.max(maxw, (#self:_strarg(arg, argtype)))
    end
    return maxw
end

function VisUtil.Args:_strarg(arg, argtype, maxw, help, default)
    self:_ensure_self()
    local s = ''
    if argtype == "nil" then
        s = ("  %s"):format(arg)
    else
        s = ("  %s <%s>"):format(arg, argtype)
    end
    if maxw ~= nil then
        s = s .. (' '):rep(maxw - #s)
    end
    if help ~= nil then
        s = s .. help
    end
    if default ~= nil then
        s = s .. (" (default %s)"):format(default)
    end
    return s .. "\n"
end

function VisUtil.Args:help(execname)
    self:_ensure_self()
    execname = execname or "<program>"
    local toplevel = "Usage: %s [arguments]\nArguments:\n%s\n"
    local topenv = "Environment variables:\n%s\n"
    local toplink = "Argument-to-Environment mapping:\n%s\n"
    local arg_str = ""
    local env_str = ""
    local link_str = ""
    local argsort, envsort, argsonly
    local maxlen

    argsort = {}
    for arg, _ in pairs(self._args) do
        if arg ~= '-h' and arg ~= '--help' then
            argsort[#argsort+1] = arg
        end
    end
    table.sort(argsort)
    argsort[#argsort+1] = '--help'
    argsort[#argsort+1] = '-h'

    envsort = {}
    for env, _ in pairs(self._envs) do
        envsort[#envsort+1] = env
    end
    table.sort(envsort)

    maxlen = self:_get_max_arglen(self._args)
    for _, arg in pairs(argsort) do
        argtype = self._args[arg]
        arg_str = arg_str .. self:_strarg(arg, argtype, maxlen,
                self._arghelp[arg], self._defaults[arg])
    end

    maxlen = self:_get_max_arglen(self._envs)
    for _, env in pairs(envsort) do
        envtype = self._envs[env]
        env_str = env_str .. self:_strarg(env, envtype, maxlen,
                self._envhelp[env])
    end

    argsort = {}
    argsonly = {}
    for arg, _ in pairs(self._links) do
        argsonly[arg] = "nil"
        argsort[#argsort+1] = arg
    end
    table.sort(argsort)
    maxlen = self:_get_max_arglen(argsonly)
    for _, arg in pairs(argsort) do
        env = self._links[arg]
        if env ~= nil then
            link_str = link_str .. ("  %s%s is equivalent to   %s\n"):format(
                arg, string.rep(' ', maxlen-#arg), env)
        end
    end

    toplevel = toplevel:format(execname, arg_str)
    if #env_str > 0 then
        topenv = topenv:format(env_str)
        toplevel = toplevel .. topenv
    end
    if #link_str > 0 then
        toplink = toplink:format(link_str)
        toplevel = toplevel .. toplink
    end

    return toplevel
end

function VisUtil.Args:parse(args)
    self:_ensure_self()
    if args == nil then
        args = Arguments
    end
    local errtab = VisUtil.Args.ERRORS
    local parsed = {}
    local parsedenv = {}
    local function doerror(errstr, arg)
        return nil, nil, errstr:format(arg)
    end
    local function ensure(value, valuetype)
        if valuetype == nil or valuetype == "nil" then
            return 1
        end
        VisUtil.Debug(("ensure(%s, %s)"):format(tostring(value), tostring(valuetype)))
        if value == nil then
            return doerror(errtab.E_NILVAL, valuetype)
        elseif valuetype == "string" then
            return value
        elseif valuetype == "number" then
            if type(value) ~= "string" or value:match("^[0-9]*$") == nil then
                return doerror(errtab.E_NOTANUM, value)
            end
            return tonumber(value)
        elseif valuetype == "boolean" or valuetype == "bool" then
            if value == "true" then return true end
            if value == "false" then return false end
            if value == "1" then return true end
            if value == "0" then return false end
            return doerror(errtab.E_NOTABOOL, value)
        else
            return doerror(errtab.E_UNKVALTP, valuetype)
        end
    end
    for arg, val in pairs(self._defaults) do
        parsed[arg] = val
    end
    for idx = 1,#args do
        for arg, val in pairs(self._args) do
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
    for env, envtype in pairs(self._envs) do
        if self._getenv(env) ~= nil then
            parsedenv[env] = ensure(self._getenv(env), envtype)
        end
    end
    for arg, env in pairs(self._links) do
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
    if parsed['-h'] or parsed['--help'] then
        print(self:help())
        os.exit()
    end
    return parsed, parsedenv, nil
end

--[[ END OF ARG API ]]

function VisUtil.wrap_coord(x, y)
    while x < 0 do x = Vis.WIDTH + x end
    while y < 0 do y = Vis.HEIGHT + y end
    while x > Vis.WIDTH do x = x - Vis.WIDTH end
    while y > Vis.HEIGHT do y = y - Vis.HEIGHT end
    return x, y
end

function VisUtil.make_emit_table()
    return {
        count = 0,
        when = 0,
        x = Vis.WIDTH / 2, y = Vis.HEIGHT / 2,
        ux = 0, uy = 0,
        depth = 0,
        s = 0, us = 0,
        radius = 1, uradius = 0,
        ds = 0, uds = 0,
        theta = 0, utheta = 0,
        life = 0, ulife = 0,
        r = 1, g = 1, b = 1,
        ur = 0, ug = 0, ub = 0,
        force = Vis.DEFAULT_FORCE,
        limit = Vis.DEFAULT_LIMIT,
        blender = Vis.BLEND_LINEAR,
        tag = 0,
    }
end

function VisUtil.center_emit_table(t, x, y, ux, uy)
    t.x = x or Vis.WIDTH / 2
    t.y = y or Vis.HEIGHT / 2
    t.ux = ux or 0
    t.uy = uy or 0
end

function VisUtil.color_emit_table(t, r, g, b, ur, ug, ub)
    t.r = r and r>1 and r/255.0 or (r or t.r)
    t.g = g and g>1 and g/255.0 or (g or t.g)
    t.b = b and b>1 and b/255.0 or (b or t.b)
    t.ur = ur and ur>1 and ur/255.0 or (ur or t.ur)
    t.ug = ug and ug>1 and ug/255.0 or (ug or t.ug)
    t.ub = ub and ub>1 and ub/255.0 or (ub or t.ub)
end

function VisUtil.emit_table(t)
    local x, y = VisUtil.wrap_coord(t.x, t.y)
    Vis.emit(Vis.flist, t.count, t.when, {
        x = x, y = y,
        ux = t.ux, uy = t.uy,
        depth = t.depth,
        s = t.s, us = t.us,
        rad = t.radius, urad = t.uradius,
        ds = t.ds, uds = t.uds,
        theta = t.theta, utheta = t.utheta,
        life = t.life, ulife = t.ulife,
        r = t.r, g = t.g, b = t.b,
        ur = t.ur, ug = t.ug, ub = t.ub,
        force = t.force,
        limit = t.limit,
        blender = t.blender,
        tag = math.floor(t.tag)
    })
end

function VisUtil.emit_table_now(t)
    local x, y = VisUtil.wrap_coord(t.x, t.y)
    Vis.emitnow(Vis.script, t.count, {
        x = x, y = y,
        ux = t.ux, uy = t.uy,
        depth = t.depth,
        s = t.s, us = t.us,
        rad = t.radius, urad = t.uradius,
        ds = t.ds, uds = t.uds,
        theta = t.theta, utheta = t.utheta,
        life = t.life, ulife = t.ulife,
        r = t.r, g = t.g, b = t.b,
        ur = t.ur, ug = t.ug, ub = t.ub,
        force = t.force,
        limit = t.limit,
        blender = t.blender,
        tag = math.floor(t.tag),
    })
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
        for k, v in pairs(o) do
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

function VisUtil.parse_midi_frame(line)
    line = line:gsub("\n", "")
    local values = {}
    for entry in line:gmatch("[^,]*[,]?") do
        entry = entry:gsub(",$", "")
        table.insert(values, entry)
    end
    return {
        id = tonumber(values[1]),
        onms = tonumber(values[2]),
        onticks = tonumber(values[3]),
        durms = tonumber(values[4]),
        durticks = tonumber(values[5]),
        track = tonumber(values[6]),
        channel = tonumber(values[7]),
        note = tonumber(values[8]),
        name = values[9],
        velocity = tonumber(values[10])
    }
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
