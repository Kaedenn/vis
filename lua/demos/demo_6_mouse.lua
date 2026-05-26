Vis = require("Vis")
VisUtil = require("visutil")
Emit = require("emit")
math = require("math")

help = {
    "Interactivity demo!",
    "Move the mouse to draw particles on the screen.",
    "Press r, g, or b to increase the color of the particles by 0.05",
    "Press R, G, or B to increase the color variance by 0.05",
    "Press 1 to cycle through the force modes (not quite visible)",
    "Press 2 to cycle through the blenders",
    "Press Up to increase particle lifetime by 50ms",
    "Press Down to decrease particle lifetime by 50ms",
    "Press Left to slow down the particles",
    "Press Right to speed up the particles"
}
print(table.concat(help, "\n"))

sym_case = {}
sym_case['1'] = '!'
sym_case['2'] = '@'
sym_case['3'] = '#'
sym_case['4'] = '$'
sym_case['5'] = '%'
sym_case['6'] = '^'
sym_case['7'] = '&'
sym_case['8'] = '*'
sym_case['9'] = '('
sym_case['0'] = ')'
sym_case['-'] = '_'
sym_case['='] = '+'
sym_case['['] = '{'
sym_case[']'] = '}'

rgb = {0, 0.2, 0.9}
urgb = {0, 0.1, 0.1}
force = Vis.DEFAULT_FORCE
blend = Vis.BLEND_LINEAR
life = 1000
ds_scale = .5
radius = 1

function clamp(val, low, high)
    if low == nil then low = 0 end
    if high == nil then high = 1 end
    if val < low then val = high
    elseif val > high then val = low end
    return val
end

Vis.on_keydown = function(key, shift)
    -- Handle shift for alphabetic keys
    if #key == 1 and shift == 0 then key = string.lower(key) end
    -- Handle shift for numeric/symbolic keys
    if sym_case[key] ~= nil and shift ~= 0 then key = sym_case[key] end

    colormod = {r = {rgb, 1},
                g = {rgb, 2},
                b = {rgb, 3},
                R = {urgb, 1},
                G = {urgb, 2},
                B = {urgb, 3}}
    if colormod[key] ~= nil then
        seq = colormod[key][1]
        val = colormod[key][2]
        seq[val] = clamp(seq[val] + 0.05)
        print("rgb  = {"..table.concat(rgb, ", ").."}")
        print("urgb = {"..table.concat(urgb, ", ").."}")
    elseif key == '1' then
        force = force + 1
        if force == Vis.NFORCES then force = 0 end
        print("force = "..VisUtil.genlua_force(force))
    elseif key == '2' then
        blend = blend + 1
        if blend == Vis.NBLENDS then blend = 0 end
        print("blend = "..VisUtil.genlua_blender(blend))
    elseif key == "Up" then
        life = clamp(life + 50, 0, 10000)
        print("life = "..life)
    elseif key == "Down" then
        life = clamp(life - 50, 0, 10000)
        print("life = "..life)
    elseif key == "Left" then
        ds_scale = clamp(ds_scale - 0.01)
        print("ds_scale = "..ds_scale)
    elseif key == "Right" then
        ds_scale = clamp(ds_scale + 0.01)
        print("ds_scale = "..ds_scale)
    elseif key == '-' then
        radius = clamp(radius - 0.5, 0, 5)
        print("radius = "..radius)
    elseif key == '+' then
        radius = clamp(radius + 0.5, 0, 5)
        print("radius = "..radius)
    else
        print("Key '"..key.."' (shift="..shift..") not bound")
    end
end

function do_emit_to(x, y)
    e = Emit:new()
    e:count(100)
    e:center(Vis.WIDTH/2, 0)
    e:radius(radius, 0)
    -- 4th root of distance * scale factor
    e:ds(math.pow(math.pow(y, 2) + math.pow(x-Vis.WIDTH/2, 2), 1/4)*ds_scale, 2)
    e:theta(math.atan2(y, x - Vis.WIDTH/2), math.pi/30)
    e:life(life, 0)
    e:color(rgb, urgb)
    e:force(force)
    e:limit(limit)
    e:blender(blend)
    e:emit_now()
end

do
    local t = VisUtil.make_emit_table()
    t.count = 0
    VisUtil.set_trace_table(t)
end

Vis.on_mousedown(do_emit_to)
Vis.on_mouseup(do_emit_to)
Vis.on_mousemove(do_emit_to)

