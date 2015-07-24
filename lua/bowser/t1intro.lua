Emit = require('emit')

local j
local k

T1.circle = Emit:new()
local circle = T1.circle
circle:radius(1)
circle:ds(2)
circle:life(SECOND/2)
circle:color(0, .4, .8, 0, .2, 0)
circle:count(100)
circle:theta(math.pi, math.pi)
function T1.emit_circle(when, x, y, ds)
    circle:ds(ds or 1)
    circle:when(when)
    for nx = x-30,x,2 do
        circle:center(nx, y)
        circle:emit()
    end
end

T1.line = Emit:new()
local line = T1.line
line:radius(2)
line:ds(1)
line:life(SECOND/3)
line:color(0, 100.0/255.0, 200.0/255.0, 0, 50.0/255.0, 0)
line:count(200)
function T1.emit_line(start, x)
    line:center(x, 0, 0, Vis.HEIGHT)
    line:theta(math.pi/2, 0.1)
    line:emit_at(start)
    line:theta(math.pi*3/2, 0.1)
    line:emit_at(start)
end

-- INTRO: PART ONE
T1.emit_circle(T1.next(), W_1_4, H_5_6)
T1.emit_circle(T1.next(), -W_1_4, H_5_6)
T1.emit_circle(T1.next(), W_1_2, H_3_4)
j = T1.next()
k = T1.next()
while j < k do
    T1.emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end

T1.emit_circle(T1.next(), W_1_2, H_2_3)
T1.emit_circle(T1.next(), W_1_2, H_3_4)

T1.emit_circle(T1.next(), W_1_2, H_2_3, circle:get('ds')/2)
T1.emit_circle(T1.next(), W_1_2, H_3_4, circle:get('ds')/2)

T1.emit_circle(T1.next(), W_1_2, H_2_3, circle:get('ds')/4)
T1.emit_circle(T1.next(), W_1_2, H_3_4, circle:get('ds')/4)

-- INTRO: PART TWO
T1.emit_circle(T1.next(), W_1_4, H_5_6)
T1.emit_circle(T1.next(), -W_1_4, H_5_6)
T1.emit_circle(T1.next(), W_1_2, H_3_4)
j = T1.next()
k = T1.next()
while j < k do
    T1.emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end
T1.emit_circle(T1.next(), W_1_4, H_5_6)
T1.emit_circle(T1.next(), -W_1_4, H_5_6)

for i = 1,3 do
    j = T1.next()
    k = T1.next()
    while j < k do
        T1.emit_line(j, W_1_2)
        j = j + Vis.frames2msec(1)
    end
end

-- INTRO: PART THREE
T1.emit_circle(T1.next(), W_1_4, H_5_6)
T1.emit_circle(T1.next(), -W_1_4, H_5_6)
T1.emit_circle(T1.next(), W_1_2, H_3_4)
j = T1.next()
k = T1.next()
while j < k do
    T1.emit_circle(j, W_1_2, H_5_6)
    j = j + Vis.frames2msec(1)
end

T1.emit_circle(T1.next(), W_1_2, H_2_3)
T1.emit_circle(T1.next(), W_1_2, H_3_4)

T1.emit_circle(T1.next(), W_1_2, H_2_3, 0.5)
T1.emit_circle(T1.next(), W_1_2, H_3_4, 0.5)

T1.emit_circle(T1.next(), W_1_2, H_2_3, 0.25)
T1.emit_circle(T1.next(), W_1_2, H_3_4, 0.25)

