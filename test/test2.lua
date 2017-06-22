package.path = package.path .. ";./test/?.lua"
package.cpath = package.cpath .. ";./build/?.so"

local sFmt = string.format 
local scene = require "scene"
local api = require "api"

math.randomseed(os.clock()*10000)
print(sFmt("===Merge Pkg Effect==Test==Start===Memory:%sK",collectgarbage("count")))
local pos_cnt = 5
local max_x = 100
local max_z = 100
local poslst1 = api.rnd_pos_lst(max_x, max_z, pos_cnt)
local poslst2 = api.rnd_pos_lst(max_x, max_z, pos_cnt)
local mArgs = {
    max_x=max_x,
    max_z=max_z,
    view_x=13,
    view_z=13,
    view_grid=1,
    aoi_type = "c",
    silent_notify = 1,
    scene_id = 1,
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
print(sFmt("----no merge pkg pos_cnt %s enter_aoi_cnt %d leave_aoi_cnt %d use time %s----",pos_cnt,scobj.m_EnterAoiCnt,scobj.m_LeaveAoiCnt,total_time))

local sync_interval = 2
local mArgs = {
    max_x=max_x,
    max_z=max_z,
    view_x=10,
    view_z=10,
    view_grid=1,
    aoi_type = "c",
    silent_notify = 1,
    scene_id = 2,
    sync_interval = sync_interval,
}

local scobj = scene.new_scene(mArgs)
local time = 1
local t1 = api.gettime()
for i,pos in ipairs(poslst1) do
    scobj:add_obj(i, pos.x, pos.z)
    time = time + 1
    if time%sync_interval == 0 then
        scobj:update()
    end
end
for i,pos in ipairs(poslst2) do
    scobj:move_obj(i, pos.x, pos.z)
    time = time + 1
    if time%sync_interval == 0 then
        scobj:update()
    end
end
for i=1,pos_cnt do
    scobj:del_obj(i)
    time = time + 1
    if time%sync_interval == 0 then
        scobj:update()
    end
end
scobj:update()
local t2 = api.gettime()
scobj:release()
local total_time = t2 - t1
print(sFmt("----merge pkg interval %d pos_cnt %s enter_aoi_cnt %d leave_aoi_cnt %d use time %s----",sync_interval,pos_cnt,scobj.m_EnterAoiCnt,scobj.m_LeaveAoiCnt,total_time))


print(sFmt("===Merge Pkg Effect==Test==End===Memory:%sK",collectgarbage("count")))