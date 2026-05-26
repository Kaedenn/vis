Vis = require("Vis")
VisUtil = require("visutil")
Letters = require("letters")
Emit = require("emit")
math = require("math")
table = require("table")

SECOND = 1000

ms = os.getenv('VIS_SEEK')
if ms ~= nil then
    VisUtil.seek_to(ms)
end

message = "I am NOT a musician!"

magnification = 3

e = Emit:new()
e:count(magnification*2)
e:center()
e:radius(4)
e:ds(0, 0.02)
e:theta(0, math.pi)
e:life(3 * SECOND)
e:color(0, 0.6, 0.7, 0, .7, 0)
e:blender(Vis.BLEND_EASING)

function emit_char(ecls, when, ord, x, y, lx, ly, zoom)
    Letters.map_fn_xy(string.char(ord):upper(), function(bx, by)
        ecls:when(when)
        ecls:center(x + zoom*(bx+lx), y + zoom*(by+ly), zoom/2, zoom/2)
        ecls:emit()
    end)
end

function emit_message(ecls, when, msg, x, y, zoom)
    for i,c in ipairs(table.pack(msg:byte(1, #msg))) do
        emit_char(ecls, when, c, x, y, (i-1) * (Letters.LETTER_WIDTH+1), 0, zoom)
    end
end

function emit_center_message(ecls, when, msg, x, y, zoom)
    local zoom = zoom or 1
    wid, len = Letters.find_extents(msg)
    emit_message(ecls, when, msg, x - wid*zoom/2, y - len*zoom/2, zoom)
end

function emit_ll_message(ecls, when, msg, x, y, zoom)
    local zoom = zoom or 1
    wid, len = Letters.find_extents(msg)
    emit_message(ecls, when, msg, x, y - len*zoom, zoom)
end

function emit_lr_message(ecls, when, msg, x, y, zoom)
    local zoom = zoom or 1
    wid, len = Letters.find_extents(msg)
    emit_message(ecls, when, msg, x - wid*zoom, y - len*zoom, zoom)
end

em = e:clone()
emit_center_message(em, 0, message, Vis.WIDTH/2, Vis.HEIGHT/2 - Vis.HEIGHT/6, 5)
em:life(2000)
emit_center_message(em, 2000, "But I tried", Vis.WIDTH/2, Vis.HEIGHT/2, 5)
em:life(500)
em:color(.2, .5, .5, 0, .5, .2)
em:theta(0, 0)
em:ds(2, 0)
emit_center_message(em, 2500, "3", Vis.WIDTH/2, Vis.HEIGHT/2 + Vis.HEIGHT/6, 4)
em:theta(math.pi, 0)
emit_center_message(em, 3000, "2", Vis.WIDTH/2, Vis.HEIGHT/2 + Vis.HEIGHT/6, 4)
em:theta(math.pi/2, 0)
emit_center_message(em, 3500, "1", Vis.WIDTH/2, Vis.HEIGHT/2 + Vis.HEIGHT/6, 4)

Vis.audio(Vis.flist, 0, "output/drum4-simple.wav")
em:blender(Vis.BLEND_PARABOLA)

e1 = em:clone()
e1:center(Vis.WIDTH/2, Vis.HEIGHT/2 + Vis.HEIGHT/6)
e1:count(2000)
e1:radius(2, 1)
e1:ds(2, 2)
e1:theta(math.pi, math.pi)
e1:life(1500, 250)

e2 = e:clone()
e2:center(Vis.WIDTH/2, Vis.HEIGHT/2 - Vis.HEIGHT/6)
e2:count(1000)
e2:life(500, 250)
e2:ds(3, 0.5)
e2:blender(Vis.BLEND_LINEAR)

e3 = e2:clone()
e3:color(.9, .9, .9, .1, .1, .1)

for i = 1,4 do
    e1:when(4000 + 2000*(i-1))
    e1:emit()
    for j = 0,3 do
        now = 4000 + 2000*(i-1) + 500*j
        e2:when(now)
        e3:when(now)
        if j == 0 then
            e3:theta(math.pi/2, math.pi)
            e3:emit()
        else
            e2:theta(0, math.pi)
            e2:emit()
            e2:theta(math.pi, math.pi)
            e2:emit()
        end
    end
end

function update_fps(time)
    fpsmsg = string.format("%.2g FPS", Vis.fps(Vis.script))
    emit_ll_message(e, time, fpsmsg, 2, Vis.HEIGHT-2, 2)
end

local step = 175
e:ds(0, 0)
e:life(step + Vis.frames2msec(1))
e:radius(1)
e:blender(Vis.BLEND_NONE)
e:color(1, 1, 1)
print(e:str())
for i = 0, 12*SECOND, step do
    emit_lr_message(e, i, i.." MS", Vis.WIDTH-2, Vis.HEIGHT-2, 2)
    Vis.callback(Vis.flist, i, Vis.script, 'update_fps('..i..')')
end

Vis.exit(Vis.flist, now + e2:get('life'))
