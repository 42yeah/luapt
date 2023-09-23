-- Custom pathtracer.
require "lib/pervasives"
local buffer = require "string.buffer"

local size = {
    w = 200,
    h = 200
}
shared_add("size", size)

-- If the pathtracer is not initialized yet, we try to initialize it.
if shared_get("initialized") == nil then
    -- Execute the initialization script once
    shade(1, 1, "ptrace_init.lua")
    shared_add("initialized", "yes")
end

shade(size.w, size.h, "pathtrace_p.lua")

local im = inventory_get("image")
save_image(im, "trace.png")
