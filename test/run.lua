package.path = package.path .. ";./test/?.lua"
package.cpath = package.cpath .. ";./build/?.so"

local sFmt = string.format 
local scene = require "scene"
local api = require "api"
print(sFmt("===test=start===Memory:%sK",collectgarbage("count")))
local scobj = scene.new_scene({max_x=100, max_z=100, view_x=10, view_z=10, view_grid=1})
-- scobj:add_obj(1, 5, 5)
-- scobj:add_obj(2, 15, 15)
-- scobj:add_obj(3, 25, 25)
-- scobj:add_obj(4, 35, 35)
-- scobj:add_obj(5, 35, 35)
-- scobj:move_obj(2, 40, 40)
-- scobj:move_obj(1, 20, 20)
-- scobj:del_obj(3)
-- scobj:del_obj(1)
local poslst = api.rnd_pos_lst(100, 100, 1000)
for i,pos in pairs(poslst) do
    scobj:add_obj(i, pos.x, pos.z)
end
scobj:release()
scobj = nil

print("---run gc---")
collectgarbage("collect")
print(sFmt("===test=end===Memory:%sK",collectgarbage("count")))