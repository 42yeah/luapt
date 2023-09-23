-- "Fragment shader" for pathtrace.lua.
require "lib/pervasives"
require "lib/camera"
require "lib/intersect"
require "lib/bvh"
local pprint = require "lib/pprint"

-- Get the pathtracing image and model from shared variables.
local im = inventory_get("image")
local model = inventory_get("model")
local bvh = inventory_get("bvh")

-- https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#UniformSampleTriangle
-- PDF in this case is 1 over triangle's area.
function sample_uniform_triangle(xi)
    local su0 = math.sqrt(xi.u)
    return vec2(1.0 - su0, xi.v * su0)
end

-- https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#UniformlySamplingaHemisphere
-- PDF in this case is 1 over hemisphere area (1 over 2 \pi).
function sample_uniform_hemisphere(xi)
    local xu = math.sqrt(1.0 - xi.u ^ 2)
    return vec3(math.cos(2.0 * math.pi * xi.v) * xu, math.sin(2.0 * math.pi * xi.v) * xu, xi.u)
end

function tri_area(tri)
    local ab = sub3(tri.b.position, tri.a.position)
    local ac = sub3(tri.c.position, tri.a.position)
    return 0.5 * len3(cross(ab, ac))
end

-- Take light source samples, together with pdf.
-- Takes in the reference point.
-- Returns the light direction and the PDF.
function emitter_sample(ref, nor)
    -- Step 1. choose a random sample
    local picked_i = math.random(bvh_emitter_count(bvh)) - 1
    local sample_uv = sample_uniform_triangle(vec2(math.random(), math.random()))

    -- Step 2. calculate target point, interpolate using barycentric coordinate
    local tri = bvh_get_emitter(bvh, picked_i)
    local sample_p = vec3(sample_uv.u, sample_uv.v, 1 - sample_uv.u - sample_uv.v)
    local tar = add3(add3(scl3(tri.a.position, sample_p.x), scl3(tri.b.position, sample_p.y)), scl3(tri.c.position, sample_p.z))

    -- Step 3. calculate distance squared, ray direction
    local dist_sqr = len3(sub3(tar, ref)) ^ 2
    local rd = nor3(sub3(tar, ref))

    -- Step 4. see if its occluded
    -- Do we need to consider this? If we generate a ray from ref to tar, then its guaranteed to hit.
    -- True, it might be occluded by other triangles. But that can be done later, and its expensive to traverse the BVH again. Maybe we'll skip this step.

    -- Step 5. evaluate PDF
    return rd, 1 / (math.abs(dot3(nor, scl3(rd, -1.0))) * tri_area(tri))
end


-- Actual pathtracing part --
local max_depth = 8

function get_sky(rd)
--     return scl3(mix3(vec3(0.99, 0.98, 1.0), vec3(0.57, 0.77, 0.89), math.min(1.0, math.abs(rd.y))), 1)
    return vec3(0.0, 0.0, 0.0)
end

-- Fake BRDF function. For now, we will just assume everything is Lambertian.
-- Since a perfect Lambertian surface is uniform, its always one over pi.
function brdf(wo, wi, p)
    return 1.0 / math.pi
end

function get_brightness(ro, rd, depth)
    local interaction = trace(bvh, model, ro, rd)
    if interaction == nil then
        -- Sample sky color
        return get_sky(rd)
    end

    local info = interaction.info

    -- If I am an emitter, then simply return my brightness
    if len3(info.emission) > 0 then
        return vec3(info.emission.x, info.emission.y, depth)
    end

    -- If I am in too deep, stop tracing, and just return my ambient values
    if depth > max_depth then
        return scl3(info.ambient, 0.001)
    end

    -- p with a slight offset; this is used to prevent intersecting with self again
    local poff = add3(interaction.position, scl3(interaction.normal, 0.01))
    local rdir = sample_uniform_hemisphere(vec2(math.random(), math.random()))

    -- BEGIN we need to translate this random_dir into space aligned with the normal.
    local fake_right = vec3(1, 0, 0)
    if math.abs(math.abs(dot3(fake_right, interaction.normal)) - 1.0) < 0.01 then
        -- Too close!
        fake_right = vec3(0, 0, 1)
    end
    local ffront = nor3(cross(interaction.normal, fake_right))
    local fright = nor3(cross(ffront, interaction.normal))

    local random_dir = add3(add3(scl3(fright, rdir.x), scl3(interaction.normal, rdir.y)), scl3(ffront, rdir.z))
    -- if the randomized direction negatively aligns with normal, flip it
    if dot3(random_dir, interaction.normal) < 0 then
        random_dir = scl3(random_dir, -1)
    end
    -- END

    local light_dir, light_pdf = emitter_sample(poff, interaction.normal)

    local total = scl3(info.ambient, 0.001)
    local random_contrib = get_brightness(poff, random_dir, depth + 1)
    local light_contrib = get_brightness(poff, light_dir, depth + 1)

    local brdf_r = brdf(scl3(rd, -1.0), random_dir, interaction.position)
    local brdf_l = brdf(scl3(rd, -1.0), light_dir, interaction.position)
    local cosine_r = math.max(0.0, dot3(interaction.normal, random_dir))
    local cosine_l = math.max(0.0, dot3(interaction.normal, light_dir))

    -- now we need to bring them into the rendering equation, which states that
--     total = add3(total, scl3(random_contrib, brdf_r * cosine_r))
    total = add3(total, random_contrib)
--     total = add3(total, scl3(light_contrib, 0.5 * brdf_l * cosine_l / light_pdf))
    return total
--     return light_dir
end

local uv = vec2(pparams.u * 2.0 - 1.0, pparams.v * 2.0 - 1.0)
local ro = vec3(0, 1, 3)
local center = vec3(0, 0.5, 0)
local right, up, front, rd = view_vectors(uv, ro, center)

-- Generate 100 samples!
local num_samples = 28
local color = vec3(0.0, 0.0, 0.0)
for i = 1, num_samples do
    color = add3(color, get_brightness(ro, rd, 0))
end

color = scl3(color, 1 / num_samples)
-- color.x = color.x ^ (1 / 2.2)
-- color.y = color.y ^ (1 / 2.2)
-- color.z = color.z ^ (1 / 2.2)
set_pixel(im, pparams.x, pparams.y, color.x, color.y, color.z)
