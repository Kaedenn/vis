Vis = require("Vis")
VisUtil = require("visutil")

print(table.concat({
    "IMPORTANT!",
    "This is an interactive test! Please either click the mouse ",
    "or press any key on the keyboard, followed by <ESC> to ",
    "close the test"
}, "\n"))

clicks = {}
keys = {}

Vis.on_mousedown = function(x, y)
    table.insert(clicks, {x, y})
    print("on_mousedown("..x..","..y..")")
end

Vis.on_mouseup = function(x, y)
    print("on_mouseup("..x..","..y..")")
end

Vis.on_mousemove = function(x, y)
    print("on_mousemove("..x..","..y..")")
end

Vis.on_keydown = function(key)
    table.insert(keys, {key})
    print("on_keydown("..key..")")
end

Vis.on_keyup = function(key)
    print("on_keyup("..key..")")
end

Vis.on_quit = function()
    did_click = 0
    print("quitting")
    for i,click in pairs(clicks) do
        did_click = 1
        print(i.." "..click[1].." "..click[2])
    end

    for i,key in pairs(keys) do
        did_click = 1
        print(i.." "..key[1])
    end

    assert(did_click == 1, "callbacks worked")
end
