
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
