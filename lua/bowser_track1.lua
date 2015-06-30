
--[[ TODO/FIXME:
-- Swap out all the 't = N, t = t + N' statements and use now(TRACK_1),
-- adv(TRACK_1, N) functions
--]]

-- INTRO: PART ONE: MAIN TRACK
t = 1233
emit_circle(t, W_1_4, H_5_6); t = t + 300
emit_circle(t, W_3_4, H_5_6); t = t + 300
emit_circle(t, W_1_2, H_3_4); t = t + 300
j = t
t = t + 1200
while j < t do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
t = t + 133

emit_circle(t, W_1_2, H_2_3); t = t + 266
emit_circle(t, W_1_2, H_3_4); t = t + 466

emit_circle(t, W_1_2, H_2_3)
t = t + 266
emit_circle(t, W_1_2, H_3_4)
t = t + 466

emit_circle(t, W_1_2, H_2_3); t = t + 266
emit_circle(t, W_1_2, H_3_4); t = t + 1466 + 500

-- INTRO: PART TWO: MAIN TRACK
emit_circle(t, W_1_4, H_5_6); t = t + 366
emit_circle(t, W_3_4, H_5_6); t = t + 366
emit_circle(t, W_1_2, H_3_4); t = t + 266
j = t
t = t + 1033
while j < t do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
t = t + 266
emit_circle(t, W_1_4, H_5_6); t = t + 233
emit_circle(t, W_3_4, H_5_6); t = t + 166

j = t
t = t + 333
while j < t do
    emit_line_v(j, W_1_2)
    j = j + 33
end
t = t + 400

j = t
t = t + 333
while j < t do
    emit_line_v(j, W_1_2)
    j = j + 33
end
t = t + 400

j = t
t = t + 333
while j < t do
    emit_line_v(j, W_1_2)
    j = j + 33
end
t = t + 400

-- INTRO: PART THREE: MAIN TRACK
emit_circle(t, W_1_4, H_5_6); t = t + 333
emit_circle(t, W_3_4, H_5_6); t = t + 333
emit_circle(t, W_1_2, H_3_4); t = t + 333
j = t
t = t + 1200
while j < t do
    emit_circle(j, W_1_2, H_5_6)
    j = j + 33
end
t = t + 133

emit_circle(t, W_1_2, H_2_3); t = t + 300
emit_circle(t, W_1_2, H_3_4); t = t + 400

emit_circle(t, W_1_2, H_2_3); t = t + 300
emit_circle(t, W_1_2, H_3_4); t = t + 400

emit_circle(t, W_1_2, H_2_3); t = t + 300
emit_circle(t, W_1_2, H_3_4); t = t + 300

