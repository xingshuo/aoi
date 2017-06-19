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

return M