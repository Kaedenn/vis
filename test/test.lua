Vis = require("Vis")

print("VIS_FPS_LIMIT: " .. Vis.FPS_LIMIT)

Vis.debug(Vis.flist)

if Vis.audio("media/Bowser.wav") then
    print("Audio loaded")
else
    print("Audio not loaded")
end
