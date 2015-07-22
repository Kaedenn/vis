Emit = require('emit')

local circle = Emit:new()
circle:radius(1)
circle:ds(2)
circle:life(SECOND/2)
circle:color(0, .4, .8, 0, .2, 0)
circle:count(100)
circle:theta(math.pi, math.pi)

function emit_circle(when, x, y, ds)
    circle:ds(ds or 1)
    circle:when(when)
    for nx = x-30,x,2 do
        circle:center(nx, y)
        circle:emit()
    end
end

local line = Emit:new()
line:radius(2)
line:ds(1)
line:life(SECOND/3)
line:color(0, 100.0/255.0, 200.0/255.0, 0, 50.0/255.0, 0)
line:count(200)
function emit_line(start, x)
    line:center(x, 0, 0, Vis.HEIGHT)
    line:theta(math.pi/2, 0.1)
    line:emit_at(start)
    line:theta(math.pi*3/2, 0.1)
    line:emit_at(start)
end

-- INTRO: PART ONE
emit_circle(T1.next(), W_1_4, H_5_6)
emit_circle(T1.next(), -W_1_4, H_5_6)
emit_circle(T1.next(), W_1_2, H_3_4)
j = T1.next()
k = T1.next()
while j < k do
    emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end

emit_circle(T1.next(), W_1_2, H_2_3)
emit_circle(T1.next(), W_1_2, H_3_4)

emit_circle(T1.next(), W_1_2, H_2_3, circle:get('ds')/2)
emit_circle(T1.next(), W_1_2, H_3_4, circle:get('ds')/2)

emit_circle(T1.next(), W_1_2, H_2_3, circle:get('ds')/4)
emit_circle(T1.next(), W_1_2, H_3_4, circle:get('ds')/4)

-- INTRO: PART TWO
emit_circle(T1.next(), W_1_4, H_5_6)
emit_circle(T1.next(), -W_1_4, H_5_6)
emit_circle(T1.next(), W_1_2, H_3_4)
j = T1.next()
k = T1.next()
while j < k do
    emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end
emit_circle(T1.next(), W_1_4, H_5_6)
emit_circle(T1.next(), -W_1_4, H_5_6)

for i = 1,3 do
    j = T1.next()
    k = T1.next()
    while j < k do
        emit_line(j, W_1_2)
        j = j + Vis.frames2msec(1)
    end
end

-- INTRO: PART THREE
emit_circle(T1.next(), W_1_4, H_5_6)
emit_circle(T1.next(), -W_1_4, H_5_6)
emit_circle(T1.next(), W_1_2, H_3_4)
j = T1.next()
k = T1.next()
while j < k do
    emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end

emit_circle(T1.next(), W_1_2, H_2_3)
emit_circle(T1.next(), W_1_2, H_3_4)

emit_circle(T1.next(), W_1_2, H_2_3, 0.5)
emit_circle(T1.next(), W_1_2, H_3_4, 0.5)

emit_circle(T1.next(), W_1_2, H_2_3, 0.25)
emit_circle(T1.next(), W_1_2, H_3_4, 0.25)

