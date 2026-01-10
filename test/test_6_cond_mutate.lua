Harness = require('harness')
Vis = require("Vis")
VisUtil = require("visutil")
Emit = require('emit')
Message = require('message')

if Vis.DEBUG < Vis.DEBUG_DEBUG then
    error("Please compile vis with -DDEBUG=2 (DEBUG_DEBUG) or greater")
end

desired_nmutates = 0
desired_nmutated = 0
desired_ntagsmod = 0

BLUE1 = {0, 0.2, 0.8, 0, 0.1, 0.2}
BLUE2 = {0, 0.2, 0.8, 0, 0.21, 0.21}
GREEN1 = {0, 0.8, 0.2, 0, 0.2, 0.1}
GREEN2 = {0, 0.8, 0.2, 0, 0.21, 0.11}

function get_mutate_debug()
    return {Vis.get_debug(Vis.script, "NUM-MUTATES"),
            Vis.get_debug(Vis.script, "PARTICLES-MUTATED"),
            Vis.get_debug(Vis.script, "PARTICLE-TAGS-MODIFIED")}
end

--[[Vis.on_quit(function()
    md = get_mutate_debug()
    nmutates, nmutated, ntagsmod = md[1], md[2], md[3]
    print("Performed "..nmutates.." mutates")
    print("Expected "..desired_nmutates.." mutates")
    print("Performed "..nmutated.." particle mutations")
    print("Expected "..desired_nmutated.." particle mutations")
    print("Performed "..ntagsmod.." modifications on particle tags")
    print("Expected "..desired_ntagsmod.." modifications on particle tags")
    assert(desired_nmutates == nmutates, "performed proper number of mutates")
    assert(desired_nmutated == nmutated, "mutated peroper number of particles")
    assert(desired_ntagsmod == ntagsmod, "modified proper number of tags")
end)]]

e = Emit:new()
e:count(1000)
e:center()
e:ds(0, 0)
e:radius(2, 0)
e:theta(math.pi, math.pi)
e:life(2000, 500)
e:color(BLUE1)

local colors = {
    BLUE1,
    GREEN1,
    BLUE2,
    GREEN2,
}

-- Vis.mutate(Vis.flist, when, Vis.MUTATE_*, factor)
-- Vis.mutate(Vis.flist, when, Vis.MUTATE_TAG_*, tag)
-- Vis.mutate(Vis.flist, when, Vis.MUTATE_*_IF, factor, Vis.MUTATE_IF_*, tag)

-- Vis.mutate(Vis.flist, when, Vis.MUTATE_*, factor)
-- VisUtil.mutate_tag(when, Vis.MUTATE_TAG_*, tag)
-- VisUtil.mutate_if(when, Vis.MUTATE_*_IF, factor, Vis.MUTATE_IF_*, tag)

em = Emit:new(e)
em:count(4)
em:radius(2, 0)
em:life(1000, 0)
em:tag(-1)
m = Message:new{emit=em}
MX, MY = Vis.WIDTH/2, Vis.HEIGHT - 50
m:set_line_spacing(1.5)
m:set_zoom(4)
m:emit_lines(1, {
    "Testing conditional mutation"
}, Vis.WIDTH/2, Vis.HEIGHT - 50)

for i = 1, 4 do
    for j = 1, 3 do
        local idx = j * 4 + i
        local x = Vis.WIDTH / 4 * i + Vis.WIDTH / 8
        local y = Vis.HEIGHT / 3 * j + Vis.HEIGHT / 6
        e:tag(idx)
        if idx % 2 == 0 then
            e:color(GREEN1)
        else
            e:color(BLUE1)
        end
        e:place(x, y, Vis.WIDTH / 100, Vis.HEIGHT / 100, Vis.WIDTH/20, Vis.WIDTH/20)
        e:emit_at(Vis.frames2msec(idx))

        m:emit_lines(1000 + Vis.frames2msec(12), {"radius=3 if even"}, MX, MY)
        Vis.mutateif(Vis.flist, Vis.frames2msec(idx) + 1000, Vis.MUTATE_SET_RADIUS_IF, Vis.MUTATE_IF_EVEN, 3, 0, 0, 0)
        Vis.mutateif(Vis.flist, Vis.frames2msec(idx) + 1000, Vis.MUTATE_SET_RADIUS_IF, Vis.MUTATE_IF_EQ, 1, 1, 0, 0, 0)

        m:emit_lines(2000 + Vis.frames2msec(12), {
            "age=0 if y<0",
            "age=1 if dist(p, center) < 10"}, MX, MY)
        Vis.mutateif(Vis.flist, Vis.frames2msec(idx) + 2000,
            Vis.MUTATE_AGE_IF, Vis.MUTATE_IF_ABOVE, 1.0, 0, 0, 0, 0, Vis.HEIGHT / 2)
        Vis.mutateif(Vis.flist, Vis.frames2msec(idx) + 2000,
            Vis.MUTATE_AGE_IF, Vis.MUTATE_IF_NEAR, 0, 0, Vis.WIDTH/10, 0, Vis.WIDTH/2, Vis.HEIGHT/2)
    end
end

--[[
for i = 1,4 do
    e:center()
    e:color(colors[i])
    e:emit_at(0)
    Vis.mutate(Vis.flist, 0, Vis.MUTATE_TAG_ADD, 2)
    desired_nmutates = desired_nmutates + 1
    desired_ntagsmod = desired_ntagsmod + e:get('count') * i
end

TOTAL_PARTICLES = e:get('count') * 4

Vis.mutate(Vis.flist, 0, Vis.MUTATE_PUSH_DX_IF, Vis.CONST_PUSH_STOP, Vis.MUTATE_IF_EQ, 2)
desired_nmutates = desired_nmutates + 1
desired_nmutated = desired_nmutated + e:get('count')

Vis.mutate(Vis.flist, 0, Vis.MUTATE_PUSH_DY_IF, Vis.CONST_PUSH_STOP, Vis.MUTATE_IF_EQ, 4)
desired_nmutates = desired_nmutates + 1
desired_nmutated = desired_nmutated + e:get('count')

Vis.mutate(Vis.flist, 0, Vis.MUTATE_SLOW_IF, 0.25, Vis.MUTATE_IF_EQ, 6)
desired_nmutates = desired_nmutates + 1
desired_nmutated = desired_nmutated + e:get('count')

Vis.mutate(Vis.flist, 0, Vis.MUTATE_SLOW_IF, 1.25, Vis.MUTATE_IF_EQ, 8)
desired_nmutates = desired_nmutates + 1
desired_nmutated = desired_nmutated + e:get('count')

Vis.mutate(Vis.flist, 1000, Vis.MUTATE_GROW_IF, 4.0, Vis.MUTATE_IF_LE, 4)
desired_nmutates = desired_nmutates + 1
desired_nmutated = desired_nmutated + e:get('count') * 2

Vis.mutate(Vis.flist, 1000, Vis.MUTATE_AGE_IF, Vis.CONST_AGE_BORN, Vis.MUTATE_IF_GE, 6)
desired_nmutates = desired_nmutates + 1
desired_nmutated = desired_nmutated + e:get('count') * 2

Vis.mutate(Vis.flist, 1500 - Vis.frames2msec(1), Vis.MUTATE_SET_DX, 2, 2)
desired_nmutates = desired_nmutates + 1
desired_nmutated = desired_nmutated + e:get('count') * 4

Vis.mutate(Vis.flist, 1500 - Vis.frames2msec(1), Vis.MUTATE_SET_DY_IF, 2, Vis.MUTATE_IF_GE, 6, 2)
desired_nmutates = desired_nmutates + 1
desired_nmutated = desired_nmutated + e:get('count') * 2
--]]

--Vis.exit(Vis.flist, 2000)
