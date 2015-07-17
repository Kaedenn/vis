
local pos_tab_s1 = {
    {W_1_2 - W_1_2/10, H_1_2 - H_1_2/10},
    {W_1_2 + W_1_2/10, H_1_2 + H_1_2/10},
    {W_1_2 - W_1_2/10, H_1_2 + H_1_2/10},
    {W_1_2 + W_1_2/10, H_1_2 - H_1_2/10},
}

for i = 1,16 do
    start = T1.next()
    stop = T1.next()
    for j = start,stop,Vis.frames2msec(1) do
        Emits.emit_circle(j, pos_tab_s1[i%4+1][1], pos_tab_s1[i%4+1][2])
    end
end

-- a b c, a b c
local pos_tab_s2 = {
    {W_1_2, H_3_4},
    {W_1_2, H_2_3},
    {W_1_2, H_1_2}
}

for _ = 1,2 do
for i = 1,3 do
    Emits.emit_circle(T1.next(), pos_tab_s2[i][1], pos_tab_s2[i][2])
    T1.next()
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
    Emits.emit_circle(T1.next(), pos_tab_s3[i][1], pos_tab_s3[i][2])
    T1.next()
end
