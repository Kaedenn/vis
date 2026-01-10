Harness = require('harness')
Vis = require("Vis")
VisUtil = require("visutil")
Emit = require('emit')

e = Emit:new()
e:count(1000)
e:center()
e:ds(1.5, 1)
e:theta(math.pi, math.pi)
e:life(2000, 500)

BLUE2 = {0, 0.2, 0.8, 0, 0.21, 0.21}
e:color(BLUE2)
e:emit_at(0)

rgb = {0, 0, 0}
nframes = 0

for i = 0,1000,Vis.frames2msec(1) do
    rgb[2] = i/2000.0 * 0.5
    rgb[3] = i/2000.0
    Vis.bgcolor(Vis.flist, i, rgb[1], rgb[2], rgb[3])
    nframes = nframes + 1
end

for i = 0,1000,Vis.frames2msec(1) do
    rgb[2] = (1000 - i) / 2000.0 * 0.5
    rgb[3] = (1000 - i) / 2000.0
    Vis.bgcolor(Vis.flist, 1000 + i, rgb[1], rgb[2], rgb[3])
    nframes = nframes + 1
end

ftype_names = {}
for name,value in pairs(Vis) do
    if name:match('FTYPE') then
        ftype_names[value] = name
    end
end

Vis.on_quit(function()
    fec = Vis.get_debug(Vis.script, 'FRAME-EMIT-COUNTS')
    for id,count in pairs(fec) do
        if count > 0 then
            print(("Vis.%s = %d"):format(ftype_names[id], count))
        end
    end
    assert(fec[Vis.FTYPE_BGCOLOR] == nframes,
           ("%d == %d"):format(fec[Vis.FTYPE_BGCOLOR], nframes))
end)

Vis.exit(Vis.flist, 3000)
