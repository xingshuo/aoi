package.path = package.path .. ";./test/?.lua"
package.cpath = package.cpath .. ";./build/?.so"

local sFmt = string.format 
local scene = require "scene"
local api = require "api"

local my_print = print 

print = function ( ... )
    -- body
end

my_print(sFmt("===test=start===Memory:%sK",collectgarbage("count")))
for i=1, 10 do
    local scobj = scene.new_scene({max_x=100, max_z=100, view_x=10, view_z=10, view_grid=1})
    local poslst = api.rnd_pos_lst(100, 100, 3000)
    for i,pos in pairs(poslst) do
        scobj:add_obj(i, pos.x, pos.z)
    end
    local poslst = api.rnd_pos_lst(100, 100, 3000)
    for i,pos in pairs(poslst) do
        scobj:move_obj(i, pos.x, pos.z)
    end
    scobj:release()
    scobj = nil
    print("---run gc---",i)
    collectgarbage("collect")
end
my_print(sFmt("===test=end===Memory:%sK",collectgarbage("count")))