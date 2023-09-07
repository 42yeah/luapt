require "lib/pervasives"

local u = pparams.u * 2.0 - 1.0
local v = pparams.v * 2.0 - 1.0

local im = inventory_get("img")
local ro = vec3(0, 0, -2)
local center = vec3(0, 0, 0)
local front = nor3(sub3(center, ro))
local right = nor3(cross(front, vec3(0, 1, 0)))
local up = nor3(cross(right, front))

local rd = nor3(add3(add3(scl3(right, u), scl3(up, v)), scl3(front, 1)))
local t = 0.01

for i = 1, 200 do
    local p = add3(ro, scl3(rd, t))
    local dist = len3(p) - 0.5
    if dist < 0.01 then
        set_pixel(im, pparams.x, pparams.y, 1.0, 0.5, 0.0)
    else
        set_pixel(im, pparams.x, pparams.y, math.abs(rd.x), math.abs(rd.y), math.abs(rd.z))
    end
    t = t + dist

    if t > 20 then
        break
    end
end


