Harness = require('harness')
Vis = require('Vis')
VisUtil = require('visutil')
Emit = require('emit')

local e = Emit:new()
e:count(100)
e:life(1000)
e:emit_at(0)

local particle_count = 0

local dump_time = Vis.frames2msec(10)
local tmpfile = os.tmpname()
Vis.callback(Vis.flist, dump_time, Vis.script, function()
    particle_count = Vis.dump_particles(Vis.script, tmpfile, "w", true)
end)

local verify_time = dump_time + Vis.frames2msec(10)
Vis.callback(Vis.flist, verify_time, Vis.script, function()
    assert(particle_count == 100, "Expected 100 particles, got " .. tostring(particle_count))
    local f = io.open(tmpfile, "r")
    local lines = 0
    if f then
        for _ in f:lines() do
            lines = lines + 1
        end
        f:close()
    end
    
    os.remove(tmpfile)

    -- We expect 101 lines: 1 header row + 100 particles
    if lines ~= 101 then
        print(package.loaded.debug.traceback())
    end
    assert(lines == 101, "Expected 101 lines in temp file, got " .. tostring(lines))

    local exported = VisUtil.export_particles()
    assert(#exported == 100, "Expected 100 exported particles, got " .. tostring(#exported))

    -- Verify some field values
    local p1 = exported[1]
    assert(p1.x >= 0 and p1.x <= 800, "p1.x out of bounds")
    assert(p1.y >= 0 and p1.y <= 600, "p1.y out of bounds")
    assert(p1.life > 0, "p1.life should be positive")

    print("Dump test passed!")
end)

Vis.exit(Vis.flist, verify_time + Vis.frames2msec(10))
