require "lib/image"
require "math"
local pprint = require "lib/pprint"

local im = Image:new(100, 100)

function shade(u, v, x, y)
    require "lib/vector.lua"

    ro = Vec4:new(0, 0, -2, 1)
    center = Vec4:new(0, 0, 0, 1)
    front = center:subtr(ro):nor3() -- most of the time we won't be needing the 4th component
    right = front:cross(Vec4:new(0, 1, 0, 1)):nor3()
    up = right:cross(front):nor3()

    rd = right:scl(u):add(up:scl(v)):add(front:scl(1)):nor3()
    t = 0.01

    for i = 1, 200 do
        p = ro:add(rd:scl(t))

        -- try to sample a sphere located at center with r=0.5
        r = 1.0
        d = p:len3() - r
        if d < 0.01 then
            -- a sphere at the center has a special case normal where the position being exactly the opposite of the normal direction
            nor = p:nor3():scl(-1)
            nor.y = -nor.y
            brightness = math.max(0.0, nor:dot3(Vec4:new(0, 1, 1, 1):nor3()))
            return Vec4:new(1.0, 0.5, 0.0, 1.0):scl(brightness)
        end
        t = t + d
        if t > 20 then
            break
        end
    end

    -- color background
    uu = math.floor(u * 5)
    vv = math.floor(v * 5)
    if (uu + vv) % 2 == 0 then
        return Vec4:new(0.1, 0.1, 0.1, 1)
    else
        return Vec4:new(0.9, 0.9, 0.9, 1)
    end

end

for y = 1, 100 do
    for x = 1, 100 do
        u = (x - 1) / 100.0 * 2.0 - 1.0
        v = (y - 1) / 100.0 * 2.0 - 1.0
        res = shade(u, v, x - 1, y - 1)
        im:pixel_vec4(x - 1, y - 1, res)
    end
end

im:save("rm.png")

print("OK")
