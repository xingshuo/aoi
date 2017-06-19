local aoi = require "aoi"

local sFmt = string.format 
print(sFmt("===test=start===Memory:%sK",collectgarbage("count")))

local view_grid = 1
local aoi_mgr = aoi.create(100,100,10,10,view_grid)
local object_tbl = {}

function obj_desc( id )
	local x,z = table.unpack(object_tbl[id])
	return sFmt("[Player %d] Pos(%d,%d)",id,x,z)
end

function enter_aoi(id, x, z)
	if object_tbl[id] then
		return
	end
	object_tbl[id] = {x,z}
	local ret = aoi_mgr:add(id, x, z)
	print(sFmt("----%s enter scene----",obj_desc(id)))
	for oid in pairs(ret) do
		print(sFmt("%s enter His View.",obj_desc(oid)))
	end
	print("")
end

function update_aoi(id, x, z)
	if not object_tbl[id] then
		return
	end
	local old_x,old_z = table.unpack(object_tbl[id])
	print(sFmt("----%s move to (%d,%d)----",obj_desc(id),x,z))
	object_tbl[id] = {x, z}
	local ret = aoi_mgr:update(id, x, z)
	if ret then
		local leave_tbl, enter_tbl = table.unpack(ret)
		for oid in pairs(leave_tbl) do
			print(sFmt("%s leave His View.",obj_desc(oid)))
		end
		for oid in pairs(enter_tbl) do
			print(sFmt("%s enter His View",obj_desc(oid)))
		end
	end
	print("")
end

function leave_aoi(id)
	if not object_tbl[id] then
		return
	end
	local x,z = table.unpack(object_tbl[id])
	local ret = aoi_mgr:delete(id)
	print(sFmt("----%s leave scene----",obj_desc(id)))
	for oid in pairs(ret) do
		print(sFmt("%s leave His View.",obj_desc(oid)))
	end
	print("")
end

enter_aoi(1, 5, 5)
enter_aoi(2, 15, 15)
enter_aoi(3, 25, 25)
enter_aoi(4, 35, 35)
-- enter_aoi(5, 35, 35)
update_aoi(2, 40,40)
update_aoi(1, 20,20)
leave_aoi(3)
leave_aoi(1)

object_tbl = nil
aoi_mgr = nil
print("---run gc---")
collectgarbage("collect")
print(sFmt("===test=end===Memory:%sK",collectgarbage("count")))
