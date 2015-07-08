Vis = require("Vis")
VisUtil = require("visutil")
Letters = require("letters")
Emit = require("emit")
math = require("math")
table = require("table")

message = "What does this button do?"

function assert_extents()
    msg1 = "Hi everyone"
    w, h = Letters.find_extents(msg1)
    cw, ch = 65, 7
    aw = #msg1 * Letters.LETTER_WIDTH + (#msg1 - 1)
    ah = Letters.LETTER_HEIGHT

    print("widths reported, calculated, desired:  "..w..", "..cw..", "..aw)
    print("heights reported, calculated, desired: "..h..", "..cw..", "..ah)
    assert(w == aw and aw == cw, "proper width reported")
    assert(h == ah and ah == ch, "proper height reported")

    msg2 = "Now with # symbols"
    w, h = Letters.find_extents(msg2)
    cw, ch = 107, 7
    aw = #msg2 * Letters.LETTER_WIDTH + (#msg2 - 1)
    ah = Letters.LETTER_HEIGHT

    print("widths reported, calculated, desired:  "..w..", "..cw..", "..aw)
    print("heights reported, calculated, desired: "..h..", "..ch..", "..ah)
    assert(w == aw and aw == cw, "proper width reported")
    assert(h == ah and ah == ch, "proper height reported")

    msg3 = "Now with # symbols\nand a longer second line"
    w, h = Letters.find_extents(msg3)
    cw, ch = 143, 15

    print("widths reported, calculated, desired:  "..w..", "..cw)
    print("heights reported, calculated, desired: "..h..", "..ch)
    assert(w == cw, "proper width reported")
    assert(h == ch, "proper height reported")
end

assert_extents()

magnification = 3

e = Emit:new()
e:count(magnification*2)
e:radius(2)
e:ds(0, 0.02)
e:theta(0, math.pi)
e:life(3000)
e:color(0, 0.8, 0.1, 0.2, .1)
e:blender(Vis.BLEND_EASING)

function emit_char(ord, x, y, lx, ly, zoom)
    Letters.map_fn_xy(string.char(ord):upper(), function(bx, by)
        e:center(x + zoom*(bx+lx), y + zoom*(by+ly), zoom/2, zoom/2)
        e:emit_now()
    end)
end

function emit_message(msg, x, y, zoom)
    for i,c in ipairs(table.pack(msg:byte(1, #msg))) do
        emit_char(c, x, y, (i-1) * (Letters.LETTER_WIDTH+1), 0, zoom)
    end
end

function emit_center_message(msg, x, y, zoom)
    local zoom = zoom or 1
    wid, len = Letters.find_extents(msg)
    emit_message(msg, x - wid*zoom/2, y - len*zoom/2, zoom)
end

emit_center_message(message, Vis.WIDTH/2, Vis.HEIGHT/2, magnification)
emit_center_message("Hi all!", Vis.WIDTH/2, Vis.HEIGHT/3, magnification)
emit_center_message("ABCDEFGHIJKLMNOPQRSTUVWXYZ", Vis.WIDTH/2, Vis.HEIGHT*3/4,
                    magnification)
emit_center_message("0 !?., 0", Vis.WIDTH/2, Vis.HEIGHT*2/3, magnification)

Vis.exit(Vis.flist, 3000)
