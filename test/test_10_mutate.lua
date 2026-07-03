Harness = require('harness')
Emit = require('emit')
VisUtil = require("visutil")
math = require("math")

Vis.on_quit(function()
    local nmutates = Vis.get_debug(Vis.script, "NUM-MUTATES")
    print(("Performed %d mutations"):format(nmutates))
    assert(nmutates == 4, "performed 4 mutations")
end)

local e = Emit:new()
e:count(1000)
e:center(Vis.WIDTH / 2, Vis.HEIGHT / 2, 0, 0, 0, 50)
e:radius(4)
e:ds(0, 0)
e:theta(0, math.pi)
e:life(5000)
e:color(1, 1, 1, 0, 0, 0)
e:blender(Vis.BLEND_NONE)
e:limit(Vis.LIMIT_SPRINGBOX)
e:emit_at(10)

Vis.mutate{
    Vis.flist,
    1000,
    func=Vis.MUTATE_ATTRACT,
    factor=-2,
    target={Vis.WIDTH/2, Vis.HEIGHT/2}
}

Vis.mutate{
    Vis.flist,
    2000,
    func=Vis.MUTATE_PUSH,
    factor=Vis.CONST_PUSH_STOP
}

Vis.mutate{
    Vis.flist,
    3000,
    func=Vis.MUTATE_ATTRACT,
    factor=1,
    target={Vis.WIDTH/2, 0}
}

Vis.mutate{
    Vis.flist,
    3500,
    func=Vis.MUTATE_ATTRACT,
    factor=-1,
    target={Vis.WIDTH/2, Vis.HEIGHT/2}
}

Vis.exit(Vis.flist, 5000)
