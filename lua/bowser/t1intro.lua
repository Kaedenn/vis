
-- INTRO: PART ONE
settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_4, H_5_6); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_3_4, H_5_6); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, T1.NextSchedule())
j = now(TRACK_1)
settime(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    T1.emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end
settime(TRACK_1, T1.NextSchedule())

T1.emit_circle(now(TRACK_1), W_1_2, H_2_3); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, T1.NextSchedule())

T1.emit_circle(now(TRACK_1), W_1_2, H_2_3, 0.5); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_2, H_3_4, 0.5); settime(TRACK_1, T1.NextSchedule())

T1.emit_circle(now(TRACK_1), W_1_2, H_2_3, 0.25); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_2, H_3_4, 0.25); settime(TRACK_1, T1.NextSchedule())

-- INTRO: PART TWO
T1.emit_circle(now(TRACK_1), W_1_4, H_5_6); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_3_4, H_5_6); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, T1.NextSchedule())
j = now(TRACK_1)
settime(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    T1.emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end
settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_4, H_5_6); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_3_4, H_5_6); settime(TRACK_1, T1.NextSchedule())

j = now(TRACK_1)
settime(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    emit_line_v(j, W_1_2)
    j = j + Vis.frames2msec(1)
end
settime(TRACK_1, T1.NextSchedule())

j = now(TRACK_1)
settime(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    emit_line_v(j, W_1_2)
    j = j + Vis.frames2msec(1)
end
settime(TRACK_1, T1.NextSchedule())

j = now(TRACK_1)
settime(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    emit_line_v(j, W_1_2)
    j = j + Vis.frames2msec(1)
end
settime(TRACK_1, T1.NextSchedule())

-- INTRO: PART THREE
T1.emit_circle(now(TRACK_1), W_1_4, H_5_6); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_3_4, H_5_6); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, T1.NextSchedule())
j = now(TRACK_1)
settime(TRACK_1, T1.NextSchedule())
while j < now(TRACK_1) do
    T1.emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end
settime(TRACK_1, T1.NextSchedule())

T1.emit_circle(now(TRACK_1), W_1_2, H_2_3); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_2, H_3_4); settime(TRACK_1, T1.NextSchedule())

T1.emit_circle(now(TRACK_1), W_1_2, H_2_3, 0.5); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_2, H_3_4, 0.5); settime(TRACK_1, T1.NextSchedule())

T1.emit_circle(now(TRACK_1), W_1_2, H_2_3, 0.25); settime(TRACK_1, T1.NextSchedule())
T1.emit_circle(now(TRACK_1), W_1_2, H_3_4, 0.25); settime(TRACK_1, T1.NextSchedule())

