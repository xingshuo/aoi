local aoi = require "aoi"
local lua_aoi = require "lua_aoi"
local api = require "api"

local sync_interval = nil

local sFmt = function ( ... )
    local str = string.format(...)
    if sync_interval then
        return string.format("\27[31m%s\27[0m",str)
    else
        return string.format("\27[33m%s\27[0m",str)
    end
end

local old_print = print 

local print = function ( ... )
    -- body
end

local PKG_SEC = 0.0001

local silent_mark = nil
function g_print(...)
    if not silent_mark then
        print(...)
    end
end

local Object = {}
Object.__index = Object

function Object:new(id, x, z, scobj)
    local o = {}
    setmetatable(o, self)
    o:init(id, x, z, scobj)
    return o
end

function Object:init(id, x, z, scobj)
    self.m_ID = id
    self.m_Pos = {x = x, z = z}
    self.m_SceneObj = scobj
end

function Object:get_pos(attr)
    if attr then
        return self.m_Pos[attr]
    else
        return self.m_Pos
    end
end

function Object:set_pos(x, z)
    self.m_Pos = {x = x, z = z}
end

function Object:enter_aoi(uuid, pkg)
    -- old_print(sFmt("%s enter %s View.",pkg,self:desc()))
    self.m_SceneObj.m_EnterAoiCnt = self.m_SceneObj.m_EnterAoiCnt + 1
end

function Object:leave_aoi(uuid, pkg)
    -- old_print(sFmt("%s leave %s View.",pkg,self:desc()))
    self.m_SceneObj.m_LeaveAoiCnt = self.m_SceneObj.m_LeaveAoiCnt + 1
end

function Object:desc()
    return sFmt("[Player %d] Pos(%d,%d)",self.m_ID,self.m_Pos.x,self.m_Pos.z)
end

function Object:release()
    self.m_SceneObj = nil
end

local Scene = {}
Scene.__index = Scene

function Scene:new(mArgs)
    local o = {}
    setmetatable(o, self)
    o:init(mArgs)
    return o
end

function Scene:init(mArgs)
    self.m_Objects = {}
    self.m_MaxX = mArgs.max_x
    self.m_MaxZ = mArgs.max_z
    self.m_ViewX = mArgs.view_x
    self.m_ViewZ = mArgs.view_z
    self.m_ViewGrid = mArgs.view_grid
    if mArgs.aoi_type == "lua" then
        self.m_CAoiMgr = lua_aoi.create(self.m_MaxX,self.m_MaxZ,self.m_ViewX,self.m_ViewZ,self.m_ViewGrid)
    else
        self.m_CAoiMgr = aoi.create(self.m_MaxX,self.m_MaxZ,self.m_ViewX,self.m_ViewZ,self.m_ViewGrid)
    end
    if mArgs.silent_notify then
        silent_mark = 1
    end
    self.m_ID = mArgs.scene_id
    self.m_AoiType = mArgs.aoi_type
    self.m_EnterAoiCnt = 0
    self.m_LeaveAoiCnt = 0
    self.m_MultiUpdateType = mArgs.multi_update_type
    self.m_DirtyObjIds = {}
    print(sFmt("---create %s---",self:desc()))
end

function Scene:desc()
    return sFmt("[Scene ID:%d Max_x:%d max_z:%d view_x:%d view_z:%d view_grid:%d AoiType:%s]",self.m_ID,self.m_MaxX,self.m_MaxZ,self.m_ViewX,self.m_ViewZ,self.m_ViewGrid,self.m_AoiType)
end

function Scene:get_obj(id)
    return self.m_Objects[id]
end

function Scene:add_obj(id, x, z)
    assert(not self.m_Objects[id])
    local obj = Object:new(id, x, z, self)
    self.m_Objects[obj.m_ID] = obj
    print(sFmt("\n----%s enter scene---",obj:desc()))
    local ret = self.m_CAoiMgr:add(obj.m_ID, x, z)
    if ret then
        for uuid in pairs(ret) do
            local o = self:get_obj(uuid)
            obj:enter_aoi(uuid, o:desc())
            o:enter_aoi(id, obj:desc())
        end
    end
end

function Scene:update()
    if self.m_MultiUpdateType == 1 then
        for id in pairs(self.m_DirtyObjIds) do
            local obj = self.m_Objects[id]
            if obj then
                local x = obj.m_Pos["x"]
                local z = obj.m_Pos["z"]
                local ret = self.m_CAoiMgr:update(id, x, z)
                if ret then
                    local leave_tbl, enter_tbl = table.unpack(ret)
                    for uuid in pairs(enter_tbl) do
                        local o = self:get_obj(uuid)
                        obj:enter_aoi(uuid, o:desc())
                        o:enter_aoi(id, obj:desc())
                    end
                    for uuid in pairs(leave_tbl) do
                        local o = self:get_obj(uuid)
                        obj:leave_aoi(uuid, o:desc())
                        o:leave_aoi(id, obj:desc())
                    end
                end
            end
        end
    elseif self.m_MultiUpdateType == 2 then
        local updatelst = {}
        for id in pairs(self.m_DirtyObjIds) do
            local obj = self.m_Objects[id]
            if obj then
                table.insert(updatelst, id)
                table.insert(updatelst, obj.m_Pos["x"])
                table.insert(updatelst, obj.m_Pos["z"])
            end
        end
        if next(updatelst) then
            local ret = self.m_CAoiMgr:update_multi(updatelst)
            for _,info in pairs(ret) do
                local id,leave_tbl,enter_tbl = table.unpack(info)
                local obj = self.m_Objects[id]
                for uuid in pairs(enter_tbl) do
                    local o = self:get_obj(uuid)
                    obj:enter_aoi(uuid, o:desc())
                    o:enter_aoi(id, obj:desc())
                end
                for uuid in pairs(leave_tbl) do
                    local o = self:get_obj(uuid)
                    obj:leave_aoi(uuid, o:desc())
                    o:leave_aoi(id, obj:desc())
                end
            end
        end
    end
    self.m_DirtyObjIds = {}
end

function Scene:move_obj(id, x, z)
    assert(self.m_Objects[id])
    local obj = self:get_obj(id)
    print(sFmt("\n----%s move to (%d,%d)----",obj:desc(),x,z))
    obj:set_pos(x, z)
    if self.m_MultiUpdateType then
        self.m_DirtyObjIds[id] = true
        return
    end
    local ret = self.m_CAoiMgr:update(id, x, z)
    if ret then
        local leave_tbl, enter_tbl = table.unpack(ret)
        for uuid in pairs(enter_tbl) do
            local o = self:get_obj(uuid)
            obj:enter_aoi(uuid, o:desc())
            o:enter_aoi(id, obj:desc())
        end
        for uuid in pairs(leave_tbl) do
            local o = self:get_obj(uuid)
            obj:leave_aoi(uuid, o:desc())
            o:leave_aoi(id, obj:desc())
        end
    end
end

function Scene:del_obj(id)
    assert(self.m_Objects[id])
    local obj = self:get_obj(id)
    self.m_Objects[id] = nil
    print(sFmt("\n----%s leave scene---",obj:desc()))
    local ret = self.m_CAoiMgr:delete(id)
    if ret then
        for oid in pairs(ret) do
            local o = self:get_obj(oid)
            o:leave_aoi(id, obj:desc())
            obj:leave_aoi(oid, o:desc())
        end
    end
    obj:release()
end

function Scene:release()
    print(sFmt("---release-scene-%d---",self.m_ID))
    self.m_Objects = {}
    self.m_CAoiMgr = nil
end

local M = {}

function M.new_scene(mArgs)
    return Scene:new(mArgs)
end

return M