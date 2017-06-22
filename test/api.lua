local twheel = require "twheel"

local M = {}

function M.rnd_pos_lst(max_x, max_z, n)
    local lst = {}
    local idx = 1
    for i=1,n do
        local x = math.random(1,max_x)
        local z = math.random(1,max_z)
        lst[idx] = {
            x = x,
            z = z,
        }
        idx = idx + 1
    end
    return lst
end

function M.table_str(mt, max_floor, cur_floor)
    cur_floor = cur_floor or 1
    max_floor = max_floor or 5
    if max_floor and cur_floor > max_floor then
        return tostring(mt)
    end
    local str
    if cur_floor == 1 then
        str = string.format("%s{\n",string.rep("--",max_floor))
    else
        str = "{\n"
    end
    for k,v in pairs(mt) do
        if type(v) == 'table' then
            v = M.table_str(v, max_floor, cur_floor+1)
        else
            if type(v) == 'string' then
                v = "'" .. v .. "'"
            end
            v = tostring(v) .. "\n"
        end
        str = str .. string.format("%s[%s] = %s",string.rep("--",cur_floor),k,v)
    end
    str = str .. string.format("%s}\n",string.rep("--",cur_floor-1))
    return str
end

function M.gettime()
    return twheel.gettime()
end

function M.sleep(s)
    os.execute(string.format("sleep %s",s))
end

return M