Harness = require('harness')
Vis = require("Vis")

_G.status = 0
Vis.on_quit(function()
    assert(status == 5, "callbacks did not call back (status is " .. tostring(status) .. ")")
end)

t1 = Vis.frames2msec(10)
t2 = Vis.frames2msec(20)
t3 = Vis.frames2msec(30)
t4 = Vis.frames2msec(40)
t5 = Vis.frames2msec(50)
t6 = Vis.frames2msec(60)

Vis.callback(Vis.flist, t1, Vis.script, 'cb()')
Vis.callback(Vis.flist, 0, Vis.script, 'print("Two at a time!\n")')

function cb()
    print("cb(), scheduling cb2")
    _G.status = 1
    Vis.callback(Vis.flist, t2, Vis.script, 'cb2()')
end

function cb2()
    _G.status = 2
    Vis.callback(Vis.flist, t3, Vis.script, 'print("Hi!\n");')
    print("cb2(), scheduling cb3 (function form)");
    Vis.callback(Vis.flist, t4, Vis.script, cb3, "arg1", 42, {a = 1})
end

function cb3(arg1, arg2, arg3)
    assert(arg1 == "arg1")
    assert(arg2 == 42)
    assert(type(arg3) == "table" and arg3.a == 1)
    _G.status = 3
    print("cb3(), scheduling anonymous functions")
    Vis.callback(Vis.flist, t5, Vis.script, function()
        print("anonymous function()")
        _G.status = 4
    end)
    Vis.callback(Vis.flist, t5, Vis.script, function(step)
        print("anonymous function with arg: " .. tostring(step))
        _G.status = _G.status + step
    end, 1)
end

Vis.exit(Vis.flist, t6)
