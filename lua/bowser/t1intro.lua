
-- INTRO: PART ONE
set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_4, H_5_6); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_3_4, H_5_6); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_2, H_3_4); set(TRACK_1, T1.NextSchedule())
j = now(TRACK_1)
set(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    Emits.emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end
set(TRACK_1, T1.NextSchedule())

Emits.emit_circle(now(TRACK_1), W_1_2, H_2_3); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_2, H_3_4); set(TRACK_1, T1.NextSchedule())

Emits.emit_circle(now(TRACK_1), W_1_2, H_2_3, 0.5); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_2, H_3_4, 0.5); set(TRACK_1, T1.NextSchedule())

Emits.emit_circle(now(TRACK_1), W_1_2, H_2_3, 0.25); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_2, H_3_4, 0.25); set(TRACK_1, T1.NextSchedule())

-- INTRO: PART TWO
Emits.emit_circle(now(TRACK_1), W_1_4, H_5_6); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_3_4, H_5_6); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_2, H_3_4); set(TRACK_1, T1.NextSchedule())
j = now(TRACK_1)
set(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    Emits.emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end
set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_4, H_5_6); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_3_4, H_5_6); set(TRACK_1, T1.NextSchedule())

j = now(TRACK_1)
set(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    Emits.emit_line_v(j, W_1_2)
    j = j + Vis.frames2msec(1)
end
set(TRACK_1, T1.NextSchedule())

j = now(TRACK_1)
set(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    Emits.emit_line_v(j, W_1_2)
    j = j + Vis.frames2msec(1)
end
set(TRACK_1, T1.NextSchedule())

j = now(TRACK_1)
set(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    Emits.emit_line_v(j, W_1_2)
    j = j + Vis.frames2msec(1)
end
set(TRACK_1, T1.NextSchedule())

-- INTRO: PART THREE
Emits.emit_circle(now(TRACK_1), W_1_4, H_5_6); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_3_4, H_5_6); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_2, H_3_4); set(TRACK_1, T1.NextSchedule())
j = now(TRACK_1)
set(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    Emits.emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end
set(TRACK_1, T1.NextSchedule())

Emits.emit_circle(now(TRACK_1), W_1_2, H_2_3); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_2, H_3_4); set(TRACK_1, T1.NextSchedule())

Emits.emit_circle(now(TRACK_1), W_1_2, H_2_3, 0.5); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_2, H_3_4, 0.5); set(TRACK_1, T1.NextSchedule())

Emits.emit_circle(now(TRACK_1), W_1_2, H_2_3, 0.25); set(TRACK_1, T1.NextSchedule())
Emits.emit_circle(now(TRACK_1), W_1_2, H_3_4, 0.25); set(TRACK_1, T1.NextSchedule())

