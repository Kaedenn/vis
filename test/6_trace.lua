Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")

Vis.debug(Vis.flist, Vis.script, "asd")

Vis.settrace(Vis.script,
    100,            -- count
    Vis.WIDTH/2,    -- x
    Vis.HEIGHT/2,   -- y
    0,              -- ux
    0,              -- uy
    1,              -- rad
    0,              -- urad
    0.25,           -- ds
    0.25,           -- uds
    0,              -- theta
    2*math.pi,      -- utheta
    100,            -- life
    0,              -- ulife
    0,              -- r
    0.2,            -- g
    0.9,            -- b
    0,              -- ur
    0.1,            -- ug
    0.1,            -- ub
    Vis.DEFAULT_FORCE, Vis.LIMIT_SPRINGBOX, Vis.BLEND_LINEAR)

