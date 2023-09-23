require "lib/pervasives"

local w = 256
local im = make_image(w, w)
inventory_add("im", im)
shade(w, w, "examples/gradient/grad_p.lua")
