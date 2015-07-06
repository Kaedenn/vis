
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
    print("quitting")
    for i,click in pairs(clicks) do
        print(i.." "..click[1].." "..click[2])
    end

    for i,key in pairs(keys) do
        print(i.." "..key[1])
    end
end
