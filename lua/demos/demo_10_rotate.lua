Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")

function rand3f(low, high)
    return math.random(low*1000, high*1000) / 100.0
end
function rand01()
    return math.random(0, 100) / 100.0
end

function emit_random(start)
    t = VisUtil.make_emit_table()
    t.count = math.random(100, 1000)
    t.when = start
    VisUtil.center_emit_table(t, math.random(0, Vis.WIDTH), math.random(0, Vis.HEIGHT),
                              math.random(-10, 10), math.random(-10, 10))
    t.ds = math.random(0, 6)
    t.uds = math.random(0, 10) / 10.0
    t.radius = math.random(5, 10) / 5.0
    t.uradius = math.random(1, 3)
    t.ds = math.random(-20, 20) / 10.0
    t.uds = math.random(-5, 5) / 5.0
    t.theta = rand3f(0, math.pi)
    t.utheta = rand3f(0, math.pi) / 10.0
    t.depth = rand3f(-25, 25)
    t.life = Vis.frames2msec(math.random(50, 200))
    t.ulife = Vis.frames2msec(math.random(0, 20))
    VisUtil.color_emit_table(t, rand01(), rand01(), rand01(),
                                rand01(), rand01(), rand01())
    forces = {Vis.DEFAULT_FORCE, Vis.FORCE_FRICTION, Vis.FORCE_GRAVITY}
    t.force = forces[math.random(1,3)]
    t.limit = Vis.LIMIT_SPRINGBOX
    t.blender = Vis.BLEND_QUADRATIC
    VisUtil.emit_table(t)
end

i=0
while i < 40 do
    emit_random(Vis.frames2msec(i*2))
    i = i + 1
end

i = 0
local total_frames = 200
while i < total_frames do
    local theta = (i / total_frames) * 180.0
    local phi = 0.0
    Vis.rotate(Vis.flist, Vis.frames2msec(i), theta, phi)
    i = i + 1
end

i = 50
while i < 200 do
    Vis.callback(Vis.flist, Vis.frames2msec(i), Vis.script,
                 'print("FPS: "..Vis.fps(Vis.script))')
    i = i + 50
end
Vis.exit(Vis.flist, Vis.frames2msec(200))
