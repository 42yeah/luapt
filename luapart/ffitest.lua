require "lib/pervasives"

-- Try to load some model

im = ffi.C.make_image(300, 300)
ffi.C.inventory_add("img", im)

ffi.C.shade(300, 300, "ffitestp.lua")

--
-- for y = 0, 300 do
--     for x = 0, 300 do
--         ffi.C.set_pixel(im, x, y, x / 300, y / 300, 0.0)
--     end
-- end
--
