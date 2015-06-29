Vis = require("Vis")
VisUtil = require("visutil")
math = require("math")

t = VisUtil.make_emit_table()
t.ds = 2
t.uds = 1
t.life = Vis.frames2msec(100)
VisUtil.color_emit_table(t, 0, 100, 200, 0, 50, 100)
t.count = 500
t.utheta = 0.25

--[[
-- Bug: simultaneous emits don't work
--  Symptom: only one of the two emits appear
--      There should be two emits in this test, one on the left half
--      pointing left, and one on the right half pointing right. However,
--      only the left emit is seen. When t.when is changed by even one
--      frame, the bug does not appear.
--  Fixed 18 Jun 2015
--]]
t.when = Vis.frames2msec(10)

VisUtil.center_emit_table(t, 200, 300)
t.theta = math.pi
VisUtil.emit_table(t)

VisUtil.center_emit_table(t, 600, 300)
t.theta = 0
VisUtil.emit_table(t)

t.when = Vis.frames2msec(100)

VisUtil.center_emit_table(t, 400, 200)
t.theta = math.pi * 3 / 2
VisUtil.emit_table(t)

VisUtil.center_emit_table(t, 400, 400)
t.theta = math.pi / 2
VisUtil.emit_table(t)

Vis.command(Vis.flist, t.life + t.when, "exit")
