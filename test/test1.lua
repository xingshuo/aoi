package.path = package.path .. ";./test/?.lua"
package.cpath = package.cpath .. ";./build/?.so"

local sFmt = string.format 
local scene = require "scene"
local api = require "api"

math.randomseed(os.clock()*10000)
print(sFmt("===test=start===Memory:%sK",collectgarbage("count")))
local pos_cnt = 10
local type_tbl = {
    ["c"] = 1,
    ["lua"] = 2,
}
local poslst1 = api.rnd_pos_lst(100, 100, pos_cnt)
local poslst2 = api.rnd_pos_lst(100, 100, pos_cnt)

for aoi_type,scid in pairs(type_tbl) do
    local total_time = 0
    local mArgs = {
        max_x=100,
        max_z=100,
        view_x=10,
        view_z=10,
        view_grid=1,
        aoi_type = aoi_type,
        silent_notify = 1,
        scene_id = scid
    }
    local scobj = scene.new_scene(mArgs)
    local t1 = api.gettime()
    for i,pos in pairs(poslst1) do
        scobj:add_obj(i, pos.x, pos.z)
    end
    for i,pos in pairs(poslst2) do
        scobj:move_obj(i, pos.x, pos.z)
    end
    for i=1,pos_cnt do
        scobj:del_obj(i)
    end
    local t2 = api.gettime()
    scobj:release()
    total_time = total_time + t2 - t1
    print(sFmt("----aoi_type: %s pos_cnt %s enter_aoi_cnt %d leave_aoi_cnt %d use time %s----",aoi_type,pos_cnt,scobj.m_EnterAoiCnt,scobj.m_LeaveAoiCnt,total_time))
end
print("---run gc---")
collectgarbage("collect")
print(sFmt("===test=end===Memory:%sK",collectgarbage("count")))