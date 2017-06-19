local aoi = require "aoi"

local sFmt = string.format 

local Object = {}
Object.__index = Object

function Object:new(id, x, z)
    local o = {}
    setmetatable(o, self)
    o:init(id, x, z)
    return o
end

function Object:init(id, x, z)
    self.m_ID = id
    self.m_Pos = {x = x, z = z}
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

function Object:enter_aoi(obj)
    print(sFmt("%s enter %s View.",obj:desc(),self:desc()))
end

function Object:leave_aoi(obj)
    print(sFmt("%s leave %s View.",obj:desc(),self:desc()))
end

function Object:desc()
    return sFmt("[Player %d] Pos(%d,%d)",self.m_ID,self.m_Pos.x,self.m_Pos.z)
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
    self.m_CAoiMgr = aoi.create(self.m_MaxX,self.m_MaxZ,self.m_ViewX,self.m_ViewZ,self.m_ViewGrid)
    print(sFmt("---create %s---",self:desc()))
end

function Scene:desc()
    return sFmt("[Scene Max_x:%d max_z:%d view_x:%d view_z:%d view_grid:%d]",self.m_MaxX,self.m_MaxZ,self.m_ViewX,self.m_ViewZ,self.m_ViewGrid)
end

function Scene:get_obj(id)
    return self.m_Objects[id]
end

function Scene:add_obj(id, x, z)
    assert(not self.m_Objects[id])
    local obj = Object:new(id, x, z)
    self.m_Objects[obj.m_ID] = obj
    print(sFmt("\n----%s enter scene---",obj:desc()))
    local ret = self.m_CAoiMgr:add(obj.m_ID, x, z)
    if ret then
        for id in pairs(ret) do
            local o = self:get_obj(id)
            obj:enter_aoi(o)
            -- o:enter_aoi(obj)
        end
    end
end

function Scene:move_obj(id, x, z)
    assert(self.m_Objects[id])
    local obj = self:get_obj(id)
    print(sFmt("\n----%s move to (%d,%d)----",obj:desc(),x,z))
    obj:set_pos(x, z)
    local ret = self.m_CAoiMgr:update(id, x, z)
    if ret then
        local leave_tbl, enter_tbl = table.unpack(ret)
        for id in pairs(enter_tbl) do
            local o = self:get_obj(id)
            obj:enter_aoi(o)
            -- o:enter_aoi(obj)
        end
        for id in pairs(leave_tbl) do
            local o = self:get_obj(id)
            obj:leave_aoi(o)
            -- o:leave_aoi(obj)
        end
    end
end

function Scene:del_obj(id)
    assert(self.m_Objects[id])
    local obj = self:get_obj(id)
    print(sFmt("\n----%s leave scene---",obj:desc()))
    self.m_Objects[id] = nil
    local ret = self.m_CAoiMgr:delete(id)
    if ret then
        for oid in pairs(ret) do
            local o = self:get_obj(oid)
            o:leave_aoi(obj)
        end
    end

end

function Scene:release()
    print("---scene release----")
    for id,obj in pairs(self.m_Objects) do
        self:del_obj(id)
    end
    self.m_CAoiMgr = nil
end

local M = {}

function M.new_scene(mArgs)
    return Scene:new(mArgs)
end

return M