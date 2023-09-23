-- Custom pathtracer. This is the initialization part.
-- We initializes three things:
-- 1. The output image.
-- 2. The model.
-- 3. The BVH.
-- Then we add them to global shared variables.
require "lib/pervasives"
require "lib/bvh"
local pprint = require "lib/pprint"
local buffer = require "string.buffer"

local size = shared_get("size")

-- Create the final result image.
local im = make_image(size.w, size.h)

-- Load the mesh to be pathtraced.
local model = make_model("cornell/CornellBox-Glossy-Floor.obj", "cornell")
local bvh = make_bvh(model)
bvh_construct(bvh, 0, 0, bvh_tri_count(bvh) - 1)

print("#emitters: ", bvh_emitter_count(bvh))

-- Add the image, model, and the BVH to shared variables.
inventory_add("image", im)
inventory_add("model", model)
inventory_add("bvh", bvh)

print("Initialization complete.")
