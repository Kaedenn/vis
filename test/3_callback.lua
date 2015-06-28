Vis = require("Vis")

Vis.callback(Vis.flist, 20, Vis.script, 'cb()')
Vis.callback(Vis.flist, 0, Vis.script, 'print("Two at a time!\n")')

function cb()
    print("cb(), scheduling cb2")
    Vis.callback(Vis.flist, 40, Vis.script, 'cb2()')
end

function cb2()
    Vis.callback(Vis.flist, 60, Vis.script, 'print("Hi!\n");')
    print("cb2()");
end

Vis.command(Vis.flist, 120, "exit")
