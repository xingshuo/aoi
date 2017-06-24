package.path = package.path .. ";./test/?.lua"
package.cpath = package.cpath .. ";./build/?.so"

local sFmt = string.format 
local scene = require "scene"
local api = require "api"

math.randomseed(os.clock()*10000)
print(sFmt("==C & Lua=Effect==Test=Start===Memory:%sK",collectgarbage("count")))
local obj_cnt = 100
local type_tbl = {
    ["c"] = 1,
    ["lua"] = 2,
}
local max_x = 137
local max_z = 206
local move_time = 1200
local poslst1 = api.rnd_pos_lst(max_x, max_z, obj_cnt)

local poslst2 = {}
for i=1,move_time do
    local l = api.rnd_pos_lst(max_x, max_z, obj_cnt)
    table.insert(poslst2, l)
end
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
    }
    local scobj = scene.new_scene(mArgs)
    local t1 = api.gettime()
    for i,pos in ipairs(poslst1) do
        scobj:add_obj(i, pos.x, pos.z)
    end
    for _,l in ipairs(poslst2) do
        for i,pos in ipairs(l) do
            scobj:move_obj(i, pos.x, pos.z)
        end
    end
    for i=1,obj_cnt do
        scobj:del_obj(i)
    end
    local t2 = api.gettime()
    scobj:release()
    local total_time = t2 - t1
    print(sFmt("----[aoi_type: %s] [obj_cnt: %s] [move_time: %s] [enter_aoi_cnt: %d leave_aoi_cnt: %d] [use time: %s]----",aoi_type,obj_cnt,move_time,scobj.m_EnterAoiCnt,scobj.m_LeaveAoiCnt,total_time))
end
poslst1 = nil
poslst2 = nil
print("---run gc---")
collectgarbage("collect")
print(sFmt("==C & Lua=Effect==Test=End===Memory:%sK",collectgarbage("count")))