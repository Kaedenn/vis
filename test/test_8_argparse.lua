Harness = require('harness')
Vis = require("Vis")
VisUtil = require("visutil")

Arguments = {
    "-a", "1",
    "-b",
    "-c", "2",
    "-d", "hi",
    "-e", "false"
}

argtypes = {
    ["-a"] = "number",
    ["-b"] = "nil",
    ["-c"] = "number",
    ["-d"] = "string",
    ["-e"] = "bool"
}

parsed = {}

Vis.on_quit(function()
end)

parser = VisUtil.Args:new()
for arg,argtype in pairs(argtypes) do
    parser:add(arg, argtype)
end

-- test: basic
parsed, parsedenv = parser:parse(Arguments)
assert(parsed['-a'] == 1)
assert(parsed['-b'] ~= nil)
assert(parsed['-c'] == 2)
assert(parsed['-d'] == "hi")
assert(parsed['-e'] == false)

-- test: basic, use _G.Arguments
parsed, parsedenv = parser:parse()
assert(parsed['-a'] == 1)
assert(parsed['-b'] ~= nil)
assert(parsed['-c'] == 2)
assert(parsed['-d'] == "hi")
assert(parsed['-e'] == false)

-- test: single non-value arg
parsed, parsedenv = parser:parse({"-b"})
assert(parsed['-a'] == nil)
assert(parsed['-b'] ~= nil)

-- test: boolean
parsed = parser:parse({"-e", "1"})
assert(parsed['-e'] == true)
parsed = parser:parse({"-e", "true"})
assert(parsed['-e'] == true)
parsed = parser:parse({"-e", "0"})
assert(parsed['-e'] == false)
parsed = parser:parse({"-e", "false"})
assert(parsed['-e'] == false)

-- test: invalid boolean
parsed, parsedenv, err = parser:parse({"-e", "notanumber"})
assert(parsed == nil)
assert(parsedenv == nil)
assert(err == VisUtil.Args.ERRORS.E_NOTABOOL:format("notanumber"))

-- test: environ
parser = VisUtil.Args:new()
parser:add_env('TEST_8_BOOL', 'bool')
parser:add_env('TEST_8_NUMBER', 'number')
parser._getenv = function(s) return ({['TEST_8_BOOL'] = "1"})[s] end
parsed, parsedenv, err = parser:parse({})
assert(#parsed == 0)
assert(err == nil)
assert(parsedenv['TEST_8_BOOL'] == true)
parser._getenv = function(s) return ({['TEST_8_NUMBER'] = "12"})[s] end
parsed, parsedenv, err = parser:parse({})
assert(#parsed == 0)
assert(err == nil)
assert(parsedenv['TEST_8_NUMBER'] == 12)

-- test: arg-env-linking
parser = VisUtil.Args:new()
args_envs_types_link = {
    {"-e", "TEST_8_E", "string"},
    {"-n", "TEST_8_N", "number"},
    {"-b", "TEST_8_B", "bool"},
    {"-x", "TEST_8_X", "nil"}
}
for _,set in pairs(args_envs_types_link) do
    parser:add(set[1], set[3])
    parser:add_env(set[2], set[3])
    parser:link_arg_env(set[1], set[2])
end
parser._getenv = function(s) return ({['TEST_8_E'] = "str"})[s] end
args, envs, err = parser:parse({"-e", "wrong"})
assert(args["-e"] == "str")


-- TODO: add environment-argument-link testing

Vis.exit(Vis.flist, 100)
