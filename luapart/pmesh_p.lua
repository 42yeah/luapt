require "lib/pervasives"
require "lib/intersect"
require "lib/bvh"


function get_sky(rd)
    return mix3(vec3(0.99, 0.98, 1.0), vec3(0.57, 0.77, 0.89), math.min(1.0, math.abs(rd.y)))
end

function trace(bvh, model, ro, rd)
    local tri, uvt = bvh_hits(bvh, 0, ro, rd)
    if uvt == nil then
        return nil
    end
    local w = 1 - uvt.x - uvt.y
    local n_bary = add3(add3(scl3(tri.b.normal, uvt.x), scl3(tri.c.normal, uvt.y)), scl3(tri.a.normal, w))
    local p = add3(add3(ro, scl3(rd, uvt.z)), scl3(n_bary, 0.01))
    -- TODO: uv should be interpolated here
    local info = model_hit_info(model, tri.a.material_id, vec2(0.0, 0.0))

    return tri, uvt, p, n_bary, info
end

function generate_ray(nor)
    -- Ray generation using rejection sampling (easy to understand, slow)
    local r = vec3(whatever(), whatever(), whatever())
    while len3(r) > 1 do
        r = vec3(whatever(), whatever(), whatever())
    end
    -- Invert it if it is on the other direction
    if dot3(r, nor) < 0 then
        return scl3(r, -1)
    end
    return r
end

local im = inventory_get("img")
local model = inventory_get("model")
local bvh = inventory_get("bvh")

local uv = vec2(pparams.u * 2.0 - 1.0, pparams.v * 2.0 - 1.0)
local ro = vec3(0.1, 0.6, 3)
local center = vec3(0, 0.5, 0)
local front = nor3(sub3(center, ro))
local right = nor3(cross(front, vec3(0, 1, 0)))
local up = nor3(cross(right, front))

local rd = nor3(add3(add3(scl3(right, uv.u), scl3(up, uv.v)), scl3(front, 1)))

local light_dir = nor3(vec3(1, 1, 1))
local tri, uvt, p, nor, info = trace(bvh, model, ro, rd)

if tri ~= nil then
    if len3(info.emission) ~= 0 then
        set_pixel(im, pparams.x, pparams.y, info.emission.x, info.emission.y, info.emission.z)
        return
    end

    local total = info.ambient

    if len3(info.diffuse) < 0.1 then
        info.diffuse = info.specular
    end

    for i = 1, 5 do
        -- Generate 5 random rays, try to hit the light source
        local r = generate_ray(nor)
        local tri_a, uvt_a, p_a, nor_a, info_a = trace(bvh, model, p, r)

        -- If it's not an emissive object, we estimate its brightness (phong.)
        if tri_a == nil then
            info_a = {
                emission = get_sky(r)
            }
        else
            if len3(info_a.diffuse) < 0.1 then
                info_a.diffuse = info_a.specular
            end
            info_a = {
                emission = scl3(info_a.diffuse, math.max(0, dot3(nor_a, vec3(0, 1, 0))))
            }
        end
        total = add3(scl3(mul3(info_a.emission, info.diffuse), math.max(0.0, dot3(r, nor))), total)
    end

    local color = scl3(total, 1 / 2)

    set_pixel(im, pparams.x, pparams.y, color.x, color.y, color.z)
else
    local c = get_sky(rd)
    set_pixel(im, pparams.x, pparams.y, c.x, c.y, c.z)
end
