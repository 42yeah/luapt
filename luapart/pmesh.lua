-- Parallel mesh renderer
require "lib/pervasives"

inventory_clear()
local model = make_model("spot.obj")
local bvh = make_bvh(model)

-- Find bounding box of the model.
local min_bbox = vec3(9999, 9999, 9999)
local max_bbox = vec3(-9999, -9999, -9999)
for i = 0, model_tri_count(model) - 1 do
    local tri = model_get_tri(model, i)
    min_bbox = min3(tri.a.position, min_bbox)
    min_bbox = min3(tri.b.position, min_bbox)
    min_bbox = min3(tri.c.position, min_bbox)

    max_bbox = max3(tri.a.position, max_bbox)
    max_bbox = max3(tri.b.position, max_bbox)
    max_bbox = max3(tri.c.position, max_bbox)
end

print(vec3str(min_bbox), vec3str(max_bbox))

local imw = 100
local imh = 100
local im = make_image(imw, imh)

inventory_add("img", im)
inventory_add("model", model)

shade(imw, imh, "pmesh_p.lua")
