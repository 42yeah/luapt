require "lib/image"
require "lib/model"
require "math"
local pprint = require "lib/pprint"

local size = 100
local im = Image:new(size, size)

function sh(u, v, x, y, w, h, handle)
    local ro = Vec4:new(0, 0, -2, 1)
    local center = Vec4:new(0, 0, 0, 1)
    local front = center:subtr(ro):nor3() -- most of the time we won't be needing the 4th component
    local right = front:cross(Vec4:new(0, 1, 0, 1)):nor3()
    local up = right:cross(front):nor3()

    local rd = right:scl(u):add(up:scl(v)):add(front:scl(1)):nor3()
    local t = 0.01

    for i = 1, 200 do
        local p = ro:add(rd:scl(t))

        -- try to sample a sphere located at center with r=0.5
        local r = 1.0
        local d = p:len3() - r
        if d < 0.01 then
            -- a sphere at the center has a special case normal where the position being exactly the opposite of the normal direction
            local nor = p:nor3():scl(-1)
            nor.y = -nor.y
            local brightness = math.max(0.0, nor:dot3(Vec4:new(0, 1, 1, 1):nor3()))

            local color = Vec4:new(1.0, 0.5, 0.0, 1.0):scl(brightness)
            set_pixel(handle, x, y, color.x, color.y, color.z, color.w)
            return
        end

        t = t + d
        if t > 20 then
            break
        end
    end

    -- color background
    local uu = math.floor(u * 5)
    local vv = math.floor(v * 5)
    if (uu + vv) % 2 == 0 then
        set_pixel(handle, x, y, 0.1, 0.1, 0.1, 1)
    else
        set_pixel(handle, x, y, 0.9, 0.9, 0.9, 1)
    end
end

res = shade(50, 50, im.handle, sh)
