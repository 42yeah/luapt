require "lib/pervasives"

-- Try to load some model

local im = make_image(300, 300)
local model = make_model("torus.obj")
inventory_add("img", im)

shade(300, 300, "ffitestp.lua")

inventory_clear()

--
-- for y = 0, 300 do
--     for x = 0, 300 do
--         ffi.C.set_pixel(im, x, y, x / 300, y / 300, 0.0)
--     end
-- end
--
