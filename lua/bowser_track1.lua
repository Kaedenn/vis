
function emit_circle(start, x, y, r, g, b, ur, ug, ub)
    local MAXLIFE = SECOND/3
    local MAXLIFE_ADJ = 0

    et = VisUtil.make_emit_table()
    VisUtil.center_emit_table(et, x or Vis.WIDTH/3, y or Vis.HEIGHT*2/3)
    et.ds = 1
    et.life = MAXLIFE
    VisUtil.color_emit_table(et, r or 0, g or 100, b or 200,
                                ur or 0, ug or 50, ub or 0);
    et.count = 100
    et.when = start
    et.theta = math.pi
    et.utheta = math.pi
    VisUtil.emit_table(et)
end

-- INTRO: PART ONE
adv(TRACK_1, 1233)
emit_circle(now(TRACK_1), W_1_4, H_5_6); adv(TRACK_1, 300)
emit_circle(now(TRACK_1), W_3_4, H_5_6); adv(TRACK_1, 300)
emit_circle(now(TRACK_1), W_1_2, H_3_4); adv(TRACK_1, 300)
j = now(TRACK_1)
adv(TRACK_1, 1200)
while j < now(TRACK_1) do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
adv(TRACK_1, 133)

emit_circle(now(TRACK_1), W_1_2, H_2_3); adv(TRACK_1, 266)
emit_circle(now(TRACK_1), W_1_2, H_3_4); adv(TRACK_1, 466)

emit_circle(now(TRACK_1), W_1_2, H_2_3)
adv(TRACK_1, 266)
emit_circle(now(TRACK_1), W_1_2, H_3_4)
adv(TRACK_1, 466)

emit_circle(now(TRACK_1), W_1_2, H_2_3); adv(TRACK_1, 266)
emit_circle(now(TRACK_1), W_1_2, H_3_4); adv(TRACK_1, 1466 + 500)

-- INTRO: PART TWO
emit_circle(now(TRACK_1), W_1_4, H_5_6); adv(TRACK_1, 366)
emit_circle(now(TRACK_1), W_3_4, H_5_6); adv(TRACK_1, 366)
emit_circle(now(TRACK_1), W_1_2, H_3_4); adv(TRACK_1, 266)
j = now(TRACK_1)
adv(TRACK_1, 1033)
while j < now(TRACK_1) do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
adv(TRACK_1, 266)
emit_circle(now(TRACK_1), W_1_4, H_5_6); adv(TRACK_1, 233)
emit_circle(now(TRACK_1), W_3_4, H_5_6); adv(TRACK_1, 166)

j = now(TRACK_1)
adv(TRACK_1, 333)
while j < now(TRACK_1) do
    emit_line_v(j, W_1_2)
    j = j + 33
end
adv(TRACK_1, 400)

j = now(TRACK_1)
adv(TRACK_1, 333)
while j < now(TRACK_1) do
    emit_line_v(j, W_1_2)
    j = j + 33
end
adv(TRACK_1, 400)

j = now(TRACK_1)
adv(TRACK_1, 333)
while j < now(TRACK_1) do
    emit_line_v(j, W_1_2)
    j = j + 33
end
adv(TRACK_1, 400)

-- INTRO: PART THREE
emit_circle(now(TRACK_1), W_1_4, H_5_6); adv(TRACK_1, 333)
emit_circle(now(TRACK_1), W_3_4, H_5_6); adv(TRACK_1, 333)
emit_circle(now(TRACK_1), W_1_2, H_3_4); adv(TRACK_1, 333)
j = now(TRACK_1)
adv(TRACK_1, 1200)
while j < now(TRACK_1) do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
adv(TRACK_1, 133)

emit_circle(now(TRACK_1), W_1_2, H_2_3); adv(TRACK_1, 300)
emit_circle(now(TRACK_1), W_1_2, H_3_4); adv(TRACK_1, 400)

emit_circle(now(TRACK_1), W_1_2, H_2_3); adv(TRACK_1, 300)
emit_circle(now(TRACK_1), W_1_2, H_3_4); adv(TRACK_1, 400)

emit_circle(now(TRACK_1), W_1_2, H_2_3); adv(TRACK_1, 300)
emit_circle(now(TRACK_1), W_1_2, H_3_4); adv(TRACK_1, 300)

