package.path = package.path .. ";./test/?.lua"
package.cpath = package.cpath .. ";./build/?.so"

local sFmt = string.format 
local scene = require "scene"
local api = require "api"

math.randomseed(os.clock()*10000)
print(sFmt("==C & Lua=Effect==Test=Start===Memory:%sK",collectgarbage("count")))
local pos_cnt = 3000
local type_tbl = {
    ["c"] = 1,
    ["lua"] = 2,
}
local max_x = 137
local max_z = 206
local poslst1 = api.rnd_pos_lst(max_x, max_z, pos_cnt)
local poslst2 = api.rnd_pos_lst(max_x, max_z, pos_cnt)

for aoi_type,scid in pairs(type_tbl) do
    local mArgs = {
        max_x=max_x,
        max_z=max_z,
        view_x=13,
        view_z=13,
        view_grid=1,
        aoi_type = aoi_type,
        silent_notify = 1,
        scene_id = scid,
        sync_interval = nil,
    }
    local scobj = scene.new_scene(mArgs)
    local t1 = api.gettime()
    for i,pos in ipairs(poslst1) do
        scobj:add_obj(i, pos.x, pos.z)
    end
    for i,pos in ipairs(poslst2) do
        scobj:move_obj(i, pos.x, pos.z)
    end
    for i=1,pos_cnt do
        scobj:del_obj(i)
    end
    local t2 = api.gettime()
    scobj:release()
    local total_time = t2 - t1
    print(sFmt("----aoi_type: %s pos_cnt %s enter_aoi_cnt %d leave_aoi_cnt %d use time %s----",aoi_type,pos_cnt,scobj.m_EnterAoiCnt,scobj.m_LeaveAoiCnt,total_time))
end
poslst1 = nil
poslst2 = nil
print("---run gc---")
collectgarbage("collect")
print(sFmt("==C & Lua=Effect==Test=End===Memory:%sK",collectgarbage("count")))