package.path = package.path .. ";./test/?.lua"
package.cpath = package.cpath .. ";./build/?.so"

local sFmt = string.format 
local scene = require "scene"
print(sFmt("===test=start===Memory:%sK",collectgarbage("count")))
local scobj = scene.new()
scobj:add_obj(1, 5, 5)
scobj:add_obj(2, 15, 15)
scobj:add_obj(3, 25, 25)
scobj:add_obj(4, 35, 35)
scobj:add_obj(5, 35, 35)
scobj:move_obj(2, 40, 40)
scobj:move_obj(1, 20, 20)
scobj:del_obj(3)
scobj:del_obj(1)
scobj:release()
scobj = nil

print("---run gc---")
collectgarbage("collect")
print(sFmt("===test=end===Memory:%sK",collectgarbage("count")))