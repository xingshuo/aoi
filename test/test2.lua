package.path = package.path .. ";./test/?.lua"
package.cpath = package.cpath .. ";./build/?.so"

local sFmt = string.format 
local scene = require "scene"
local api = require "api"

math.randomseed(os.clock()*10000)
print(sFmt("===Merge Pkg Effect==Test==Start===Memory:%sK",collectgarbage("count")))
local obj_cnt = 200
local max_x = 100
local max_z = 100
local poslst1 = api.rnd_pos_lst(max_x, max_z, obj_cnt)
local poslst2 = {}
local move_time = 400
for i=1,move_time do
    local l = api.rnd_pos_lst(max_x, max_z, obj_cnt)
    table.insert(poslst2, l)
end
local sync_interval = 800
local update_types = {0,1,2}

for scid,uptype in ipairs(update_types) do
    if uptype == 0 then
        uptype = nil
    end
    local mArgs = {
        max_x=max_x,
        max_z=max_z,
        view_x=10,
        view_z=10,
        view_grid=1,
        aoi_type = "c",
        silent_notify = 1,
        scene_id = scid,
        multi_update_type = uptype,
    }

    local scobj = scene.new_scene(mArgs)
    local time = 1
    local t1 = api.gettime()
    for i,pos in ipairs(poslst1) do
        scobj:add_obj(i, pos.x, pos.z)
        time = time + 1
    end
    for _,l in ipairs(poslst2) do
        for i,pos in ipairs(l) do
            scobj:move_obj(i, pos.x, pos.z)
            time = time + 1
            if uptype and time%sync_interval == 0 then
                scobj:update()
            end
        end
    end
    for i=1,obj_cnt do
        scobj:del_obj(i)
        time = time + 1
    end
    if uptype then
        scobj:update()
    end
    local t2 = api.gettime()
    scobj:release()
    local total_time = t2 - t1
    print(sFmt("----merge pkg type:%s interval:%d obj_cnt:%s move_time:%s enter_aoi_cnt %d leave_aoi_cnt %d use time %s----",uptype,sync_interval,obj_cnt,move_time,scobj.m_EnterAoiCnt,scobj.m_LeaveAoiCnt,total_time))
end

print(sFmt("===Merge Pkg Effect==Test==End===Memory:%sK",collectgarbage("count")))