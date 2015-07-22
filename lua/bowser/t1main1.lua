do

local e = Emit:new()
e:count(100)
e:radius(2)
e:ds(1)
e:life(SECOND/3, SECOND/6)
e:color(0, .4, .8, 0, .2, 0)
e:theta(math.pi, math.pi)

--[[ FIXME
-- The 2x2 design below really does look awful
--]]
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
        e:center(pos_tab_s1[i%4+1][1], pos_tab_s1[i%4+1][2])
        e:emit_at(j)
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
        start = T1.next()
        stop = T1.next()
        e:center(pos_tab_s2[i][1], pos_tab_s2[i][2])
        for j = start,stop,Vis.frames2msec(1) do
            e:emit_at(j)
        end
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
    start = T1.next()
    stop = T1.next()
    e:center(pos_tab_s3[i][1], pos_tab_s3[i][2])
    for j = start,stop,Vis.frames2msec(1) do
        e:emit_at(j)
    end
end

end
