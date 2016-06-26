Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")

random = math.random
function rand3f(low, high)
    return random(low*1000, high*1000) / 100.0
end
function rand01()
    return random(0, 100) / 100.0
end

function emit_random(start)
    t = VisUtil.make_emit_table()
    t.count = random(100, 1000)
    t.when = start
    if rand01() > 0.25 then
        -- circle emit
        VisUtil.center_emit_table(t, random(0, Vis.WIDTH), random(0, Vis.HEIGHT),
                                  random(-10, 10), random(-10, 10))
        t.theta = rand3f(0, math.pi)
        t.utheta = rand3f(0, math.pi) / 10.0
    elseif rand01() > 0.5 then
        VisUtil.center_emit_table(t, 0, 0, Vis.WIDTH, 0)
        t.theta = math.pi/2
    elseif rand01() > 0.5 then
        VisUtil.center_emit_table(t, 0, Vis.HEIGHT, Vis.WIDTH, 0)
        t.theta = 3*math.pi/2
    elseif rand01() > 0.5 then
        VisUtil.center_emit_table(t, 0, 0, 0, Vis.HEIGHT)
        t.theta = 0
    else
        VisUtil.center_emit_table(t, Vis.WIDTH, 0, 0, Vis.HEIGHT)
        t.theta = math.pi
    end
    t.ds = random(0, 6)
    t.uds = random(0, 10) / 10.0
    t.radius = random(2, 10) / 2.0
    t.uradius = random(2, 10) / 4.0
    t.ds = random(-20, 20) / 10.0
    t.uds = random(-5, 5) / 5.0
    t.life = Vis.frames2msec(random(50, 200))
    t.ulife = Vis.frames2msec(random(0, 20))
    VisUtil.color_emit_table(t, rand01(), rand01(), rand01(),
                                rand01(), rand01(), rand01())
    forces = {Vis.DEFAULT_FORCE, Vis.FORCE_FRICTION, Vis.FORCE_GRAVITY}
    t.force = forces[random(1,3)]
    t.limit = Vis.LIMIT_SPRINGBOX
    t.blender = Vis.BLEND_QUADRATIC
    VisUtil.emit_table(t)
end

i=0
while i < 200 do
    emit_random(Vis.frames2msec(i*2))
    i = i + 1
end
i = 50
while i < 600 do
    Vis.callback(Vis.flist, Vis.frames2msec(i), Vis.script,
                 'print("FPS: "..Vis.fps(Vis.script))')
    i = i + 50
end
Vis.exit(Vis.flist, Vis.frames2msec(600))
Vis.on_quit(function()
    print("exiting now")
end)
