Vis = require("Vis")

t1 = Vis.frames2msec(20)
t2 = Vis.frames2msec(40)
t3 = Vis.frames2msec(60)
t4 = Vis.frames2msec(120)

Vis.callback(Vis.flist, t1, Vis.script, 'cb()')
Vis.callback(Vis.flist, 0, Vis.script, 'print("Two at a time!\n")')

function cb()
    print("cb(), scheduling cb2")
    Vis.callback(Vis.flist, t2, Vis.script, 'cb2()')
end

function cb2()
    Vis.callback(Vis.flist, t3, Vis.script, 'print("Hi!\n");')
    print("cb2()");
end

Vis.command(Vis.flist, t4, "exit")
