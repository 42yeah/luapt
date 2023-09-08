-- Parallel mesh renderer
require "lib/pervasives"

local model = make_model("torus.obj")
local imw = 360
local imh = 360
local im = make_image(imw, imh)

inventory_add("img", im)
inventory_add("model", model)

shade(imw, imh, "pmesh_p.lua")
