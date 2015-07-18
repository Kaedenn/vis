
do

debug = require('debug')
local fn = function() end
local filename = debug.getinfo(fn).short_src

print("Starting test "..filename)

end
