require "lib/pervasives"

im = ffi.C.inventory_get("img")
ffi.C.set_pixel(im, pparams.x, pparams.y, pparams.x / 300, pparams.y / 300, 0.0)
