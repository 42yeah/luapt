-- Parallel mesh renderer
require "lib/pervasives"
require "lib/intersect"
require "lib/bvh"

inventory_clear()
local model = make_model("sponza/sponza.obj", "sponza")
local bvh = make_bvh(model)

-- Print root bbox
local root_bbox = bvh_get_node(bvh, 0)
print(vstr(root_bbox.bbox.min), vstr(root_bbox.bbox.max))

bvh_construct(bvh, 0, 0, bvh_tri_count(bvh) - 1)

local imw = 200
local imh = 200
local im = make_image(imw, imh)

inventory_add("img", im)
inventory_add("model", model)
inventory_add("bvh", bvh)

shade(imw, imh, "pmesh_p.lua")

save_image(im, "cornell.png")
