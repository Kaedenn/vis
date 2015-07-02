
function emit_border_l(start, r, g, b, ur, ug, ub)
    local MAXLIFE = SECOND/2
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, 0, 0, 0, Vis.HEIGHT)
    et.ds = 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0)
    et.count = 200
    et.when = start
    et.theta = 0
    et.utheta = 0.2
    VisUtil.emit_table(et)
end

function emit_border_r(start, r, g, b, ur, ug, ub)
    local MAXLIFE = SECOND/2
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, Vis.WIDTH, 0, 0, Vis.HEIGHT)
    et.ds = 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0)
    et.count = 200
    et.when = start
    et.theta = math.pi
    et.utheta = 0.2
    VisUtil.emit_table(et)
end

j = now(TRACK_3)
adv(TRACK_3, 5854)
while j < now(TRACK_3) do
    emit_border_l(j, .8, .2, .2, .2, .1, .1)
    emit_border_r(j, .8, .2, .2, .2, .1, .1)
    j = j + 33
end
adv(TRACK_3, 66)

j = now(TRACK_3)
adv(TRACK_3, 6150)
while j < now(TRACK_3) do
    emit_border_l(j, 1, .2, .2, 0, .1, .1)
    emit_border_r(j, 1, .2, .2, 0, .1, .1)
    j = j + 33
end
adv(TRACK_3, 198)

j = now(TRACK_3)
adv(TRACK_3, 1441)
while j < now(TRACK_3) do
    emit_border_l(j, 1, .2, .2, 0, .1, .1)
    emit_border_l(j, 1, .2, .2, 0, .1, .1)
    emit_border_r(j, 1, .2, .2, 0, .1, .1)
    emit_border_r(j, 1, .2, .2, 0, .1, .1)
    j = j + 33
end
adv(TRACK_3, 33)
