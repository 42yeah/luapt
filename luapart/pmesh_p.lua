require "lib/pervasives"
require "lib/intersect"


function get_sky(rd)
    return mix3(vec3(0.99, 0.98, 1.0), vec3(0.57, 0.89, 0.77), math.min(1.0, math.abs(rd.y)))
end

local im = inventory_get("img")
local bvh = inventory_get("bvh")

local uv = vec2(pparams.u * 2.0 - 1.0, pparams.v * 2.0 - 1.0)
local ro = vec3(2, 2, 2)
local center = vec3(0, 0, 0)
local front = nor3(sub3(center, ro))
local right = nor3(cross(front, vec3(0, 1, 0)))
local up = nor3(cross(right, front))

local rd = nor3(add3(add3(scl3(right, uv.u), scl3(up, uv.v)), scl3(front, 1)))

function hits(node_idx, ro, rd)
    local n = bvh_get_node(bvh, node_idx)
    if intersect_box(ro, rd, n.bbox) == nil then
        return nil
    end

    local closest_tri = nil
    local closest_uvt = nil

    -- If hits, and is leave node, then we iterate over the triangles
    -- Otherwise, we recurse into the leave node
    if n.l == 0 and n.r == 0 then
        for i = n.start, n.start + n.size - 1 do
            local tri = bvh_get_tri(bvh, i)
            local uvt = intersect(ro, rd, tri, 0.01, 100.0)

            if uvt ~= nil then
                if closest_uvt == nil or uvt.z < closest_uvt.z then
                    closest_tri = tri
                    closest_uvt = uvt
                end
            end
        end
    else
        local tri_l, uvt_l = hits(n.l, ro, rd)
        local tri_r, uvt_r = hits(n.r, ro, rd)
        if uvt_l == nil then
            return tri_r, uvt_r
        elseif uvt_r == nil then
            return tri_l, uvt_l
        elseif uvt_l.z < uvt_r.z then
            return tri_l, uvt_l
        end
        return tri_r, uvt_r
    end

    return closest_tri, closest_uvt
end

local light_dir = nor3(vec3(1, 1, 1))
local tri, uvt = hits(0, ro, rd)

if uvt ~= nil then
    -- VERY simple flat shading
    local val = math.min(1.0, math.max(0.0, dot3(light_dir, tri.a.normal)) + 0.2)
    local color = scl3(vec3(1.0, 0.5, 0.0), val)

    local w = 1 - uvt.x - uvt.y
    local n_bary = add3(add3(scl3(tri.b.normal, uvt.x), scl3(tri.c.normal, uvt.y)), scl3(tri.a.normal, w))

    local p = add3(add3(ro, scl3(rd, uvt.z)), scl3(n_bary, 0.01))
    local refl = add3(add3(rd, n_bary), n_bary)
    local sky = min3(vec3(1.0, 1.0, 1.0), get_sky(refl))

    set_pixel(im, pparams.x, pparams.y, sky.x, sky.y, sky.z)
else
    local c = get_sky(rd)
    set_pixel(im, pparams.x, pparams.y, c.x, c.y, c.z)
end
