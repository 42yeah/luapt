require "lib/image"
require "lib/model"
require "math"
local pprint = require "lib/pprint"

local size = 10
local im = Image:new(size, size)
local model = Model:new("torus.obj")
print("Model loaded: ", model.path, ". #triangles:", model:tri_count())

function shade(u, v, x, y)
    local ro = Vec4:new(2, 1.2, 2.5, 1)
    local center = Vec4:new(0, 0.0, 0.0, 1)
    local front = center:subtr(ro):nor3() -- most of the time we won't be needing the 4th component
    local right = front:cross(Vec4:new(0, 1, 0, 1)):nor3()
    local up = right:cross(front):nor3()

    local rd = right:scl(u):add(up:scl(v)):add(front:scl(2)):nor3()

    if u == 0 and y == 0 then
        pprint(rd)
    end

    local closest_tri = nil
    local closest_t = -1

    for i = 1, model:tri_count() do
        local tri = model:tri(i)
        local t = intersect(ro, rd, tri, 0.01, 100.0)
        if t ~= nil and (t < closest_t or closest_tri == nil) then
            closest_tri = tri
            closest_t = t
        end
    end

    if closest_tri == nil then
        -- color background
        local uu = math.floor(u * 5)
        local vv = math.floor(v * 5)
        if (uu + vv) % 2 == 0 then
            return Vec4:new(0.1, 0.1, 0.1, 1)
        else
            return Vec4:new(0.9, 0.9, 0.9, 1)
        end
    end

    local rd = Vec4:new(1, 0.5, 0.2, 1):nor3()
    local ambient = 0.2
    local diffuse = math.max(0, rd:dot3(closest_tri.a.normal))

    return Vec4:new(1.0, 0.5, 0.0, 1):scl(ambient + diffuse)
end

for y = 1, size do
    print(y)
    for x = 1, size do
        u = (x - 1) / size * 2.0 - 1.0
        v = (y - 1) / size * 2.0 - 1.0
        res = shade(u, v, x - 1, y - 1)
        im:pixel_vec4(x - 1, y - 1, res)
    end
end

im:save("torus.png")
