-- Parallel mesh renderer
require "lib/pervasives"
require "lib/intersect"
require "lib/bvh"

inventory_clear()
local model = make_model("cornell/CornellBox-Sphere.obj", "cornell")
local bvh = make_bvh(model)

bvh_construct(bvh, 0, 0, bvh_tri_count(bvh) - 1)

local imw = 100
local imh = 100
local im = make_image(imw, imh)

inventory_add("img", im)
inventory_add("model", model)
inventory_add("bvh", bvh)

shade(imw, imh, "pmesh_p.lua")

save_image(im, "suzanne.png")
