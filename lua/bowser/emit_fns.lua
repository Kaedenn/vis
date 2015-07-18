do

Emits = {}

function Emits.set_emit_left(et)
    VisUtil.center_emit_table(et, 0, 0, 0, Vis.HEIGHT)
    et.theta = 0
end

function Emits.set_emit_right(et)
    VisUtil.center_emit_table(et, Vis.WIDTH, 0, 0, Vis.HEIGHT)
    et.theta = math.pi
end

function Emits.set_emit_top(et)
    VisUtil.center_emit_table(et, 0, 0, Vis.WIDTH, 0)
    et.theta = math.pi/2
end

function Emits.set_emit_bottom(et)
    VisUtil.center_emit_table(et, 0, Vis.HEIGHT, Vis.WIDTH, 0)
    et.theta = 3*math.pi/2
end

function Emits.emit_circle(start, x, y, ds)
    local MAXLIFE = SECOND/3
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, x or Vis.WIDTH/3, y or Vis.HEIGHT*2/3)
    et.radius = 2
    et.ds = ds or 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, 0, 100, 200, 0, 50, 0);
    et.count = 100
    et.when = start
    et.theta = math.pi
    et.utheta = math.pi
    VisUtil.emit_table(et)
end

--[[ Unused
function Emits.emit_spiral(steps)
    local MAXLIFE = 10/3*SECOND
    local MAXLIFE_ADJ = 1/3*SECOND

    et = VisUtil.make_emit_table()
    center_emit_table(et)
    et.radius = 2
    et.ds = 2
    et.uds = 1.75
    et.life = MAXLIFE
    et.ulife = MAXLIFE_ADJ
    VisUtil.color_emit_table(et, 0, 100, 200, 0, 10, 50)
    for i = 0, steps do
        et.count = 1000
        et.when = i
        et.theta = 2*math.pi * i / steps
        et.utheta = 2*math.pi / steps
        VisUtil.emit_table(et)
    end
    return steps + MAXLIFE + MAXLIFE_ADJ
end
--]]

function Emits.emit_line_v(start, x, r, g, b, ur, ug, ub)
    local MAXLIFE = SECOND/3
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, x, 0, 0, Vis.HEIGHT)
    et.radius = 2
    et.ds = 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0)
    et.count = 200
    et.when = start
    et.theta = math.pi/2
    et.utheta = 0.1
    VisUtil.emit_table(et)
    et.theta = math.pi*3/2
    et.utheta = 0.1
    VisUtil.emit_table(et)
    return start + MAXLIFE + MAXLIFE_ADJ
end

function Emits.emit_line_h(start, y, r, g, b, ur, ug, ub)
    local MAXLIFE = SECOND/3
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, 0, y, Vis.WIDTH, 0)
    et.radius = 2
    et.ds = 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0)
    et.count = 200
    et.when = start
    et.theta = 0
    VisUtil.emit_table(et)
    et.theta = math.pi
    VisUtil.emit_table(et)
    return start + MAXLIFE + MAXLIFE_ADJ
end

return Emits

end
