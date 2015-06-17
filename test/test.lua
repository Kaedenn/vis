Vis = require("Vis")

print("VIS_FPS_LIMIT: " .. Vis.FPS_LIMIT)

Vis.debug(Vis.flist)

if Vis.audio("media/Bowser.wav") then
    print("Audio loaded")
else
    print("Audio not loaded")
end

i = 0
while i < 100 do
    theta = 6.28 * i / 100
    Vis.emit(Vis.flist, 1000 * (50 - i) / 50, i, 400, 300, 0, 0, 1, 0, 1, 1, theta, 6.28 / 100, 100, 10, 0, 100, 200, 0, 10, 50, 0, 0, Vis.BLEND_LINEAR)
    i = i + 1
end
