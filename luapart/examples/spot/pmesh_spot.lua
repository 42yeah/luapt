-- Parallel mesh renderer
require "lib/pervasives"
require "lib/intersect"
require "lib/bvh"

inventory_clear()
local model = make_model("spot.obj", "cornell")
local bvh = make_bvh(model)

-- Force a spot texture?
local spot = load_image("spot_texture.png")

bvh_construct(bvh, 0, 0, bvh_tri_count(bvh) - 1)

local imw = 200
local imh = 200
local im = make_image(imw, imh)

inventory_add("img", im)
inventory_add("model", model)
inventory_add("bvh", bvh)
inventory_add("spot", spot)

shade(imw, imh, "pmesh_p.lua")

save_image(im, "suzanne.png")
