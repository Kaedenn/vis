
local pos_tab_s1 = {
    {W_1_2 - W_1_2/10, H_1_2 - H_1_2/10},
    {W_1_2 + W_1_2/10, H_1_2 + H_1_2/10},
    {W_1_2 - W_1_2/10, H_1_2 + H_1_2/10},
    {W_1_2 + W_1_2/10, H_1_2 - H_1_2/10},
}

for i = 1,16 do
    if i ~= 1 then settime(TRACK_1, T1.NextSchedule()) end
    Emits.emit_circle(now(TRACK_1), pos_tab_s1[i%4+1][1], pos_tab_s1[i%4+1][2])
    settime(TRACK_1, T1.NextSchedule())
end

-- a b c, a b c
local pos_tab_s2 = {
    {W_1_2, H_3_4},
    {W_1_2, H_2_3},
    {W_1_2, H_1_2}
}

for _ = 1,2 do
for i = 1,3 do
    settime(TRACK_1, T1.NextSchedule())
    Emits.emit_circle(now(TRACK_1), pos_tab_s2[i][1], pos_tab_s2[i][2])
    settime(TRACK_1, T1.NextSchedule())
end
end

-- 1, 2, 3, 4
local pos_tab_s3 = {
    {W_1_3, H_1_2},
    {W_2_3, H_1_2},
    {W_1_4, H_1_3},
    {W_3_4, H_1_3},
}

for i = 1,4 do
    settime(TRACK_1, T1.NextSchedule())
    Emits.emit_circle(now(TRACK_1), pos_tab_s3[i][1], pos_tab_s3[i][2])
    settime(TRACK_1, T1.NextSchedule())
end
