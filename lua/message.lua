Letters = require("letters")
Emit = require("emit")
utf8 = require("utf8")

--[[
-- Multi-character emit object
--
-- Message:new takes an optional configuration object with the following
-- supported keys, all optional:
--  emit            Emit class to use
--  line_spacing    Distance between lines as a multiple of line height
--                  (default: 1.5)
--  zoom            Size multiplier (default: 2)
--]]
Message = {}
function Message:new(obj)
    local o = {
        _t = {
            emit = nil,
            line_spacing = 1.5,
            zoom = 2
        }
    }
    setmetatable(o, self)
    self.__index = self
    o:configure(obj or {})
    return o
end

--[[ Update local configuration from a key-value table ]]
function Message:configure(obj)
    for key, val in pairs(obj) do
        self:set(key, val)
    end
end

--[[ Direct modification of the configuration table ]]
function Message:set(key, val) self._t[key] = val end
function Message:get(key, default) return self._t[key] or default end
function Message:set_emit(obj) self:set("emit", obj) end
function Message:set_line_spacing(value) self:set("line_spacing", value) end
function Message:set_zoom(value) self:set("zoom", value) end

--[[ Ensure we aren't calling an emit function without an emit object ]]
function Message:_ensure_emit()
    assert(self._t.emit ~= nil, "attempt to invoke Message without emit object")
end

--[[ Emit a single character at the given location
--
-- If when is nil, then e:emitnow() is called instead of e:emit() ]]
function Message:emit_char(when, char, anchorx, anchory, letterx, lettery)
    self:_ensure_emit()
    local e = self:get("emit"):copy() -- we're gonna modify it, so copy it first
    local zoom = self:get("zoom")
    Letters.map_fn_xy(char:upper(), function(basex, basey)
        local charx = anchorx + zoom*(basex+letterx)
        local chary = anchory + zoom*(basey+lettery)
        local adjy = #Letters[char:upper()] - Letters.LETTER_HEIGHT
        e:center(charx, chary - adjy, zoom/2, zoom/2)
        if when then
            e:emit(when)
        else
            e:emitnow()
        end
    end)
end

--[[ Emit a sequence of letters starting at the given location ]]
function Message:emit_message(when, message, anchorx, anchory)
    self:_ensure_emit()
    local idx = 1
    for _, ord in utf8.codes(message) do
        local char = string.char(ord)
        local letterx = (idx-1)*(Letters.LETTER_WIDTH+1)
        self:emit_char(when, char, anchorx, anchory, letterx, 0, zoom)
        idx = idx + 1
    end
end

--[[ Emit a message as above, but center aligned instead of left aligned ]]
function Message:emit_center_message(when, message, anchorx, anchory)
    self:_ensure_emit()
    local zoom = self:get("zoom")
    local width, length = Letters.find_extents(message)
    local messagex = anchorx - width*zoom/2
    local messagey = anchory - length*zoom/2
    self:emit_message(when, message, messagex, messagey)
end

--[[ Emit a table of strings on consecutive lines, centered ]]
function Message:emit_lines(when, lines, anchorx, anchory)
    self:_ensure_emit()
    local zoom = self:get("zoom")
    local line_height = self:get("line_spacing") * Letters.LETTER_HEIGHT * zoom
    for idx, line in ipairs(lines) do
        local liney = anchory + (idx-1)*line_height
        self:emit_center_message(when, line, anchorx, liney)
    end
end

--[[ Emit arbitrary text (of possibly multiple lines) ]]
function Message:emit_text(when, text, anchorx, anchory)
    self:_ensure_emit()
    local zoom = self:get("zoom")
    local line_height = self:get("line_spacing") * Letters.LETTER_HEIGHT * zoom
    local liney = anchory
    for line in text:gmatch("[^\n]*[\n]?") do
        self:emit_center_message(when, line:gsub("\n", ""), anchorx, liney)
        liney = liney + line_height
    end
end

return Message
