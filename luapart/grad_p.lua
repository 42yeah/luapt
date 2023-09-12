require "lib/pervasives"

local im = inventory_get("im")
set_pixel(im, pparams.x, pparams.y, pparams.u * 2.0, pparams.v * 2.0, 0.0)
