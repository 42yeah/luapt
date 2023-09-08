require "lib/pervasives"
require "lib/intersect"

local im = inventory_get("img")
local model = inventory_get("model")

local uv = vec2(pparams.u * 2.0 - 1.0, pparams.v * 2.0 - 1.0)
local ro = vec3(0, 2, -3)
local center = vec3(0, 0, 0)
local front = nor3(sub3(center, ro))
local right = nor3(cross(front, vec3(0, 1, 0)))
local up = nor3(cross(right, front))

local rd = nor3(add3(add3(scl3(right, uv.u), scl3(up, uv.v)), scl3(front, 1)))

local mtc = model_tri_count(model)
for i = 0, mtc do
    local tri = model_get_tri(model, i)
    if intersect(ro, rd, tri, 0.01, 100.0) then
        set_pixel(im, pparams.x, pparams.y, 1, 0, 1)
        return
    end
end

function get_sky(rd)
    return mix3(vec3(0.99, 0.98, 1.0), vec3(0.65, 0.86, 0.98), math.abs(rd.y))
end

c = get_sky(rd)
set_pixel(im, pparams.x, pparams.y, c.x, c.y, c.z)
