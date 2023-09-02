require "lib/image"
require "lib/model"
require "math"
local pprint = require "lib/pprint"

local imw = 720
local imh = 720
local im = Image:new(imw, imh)

-- Snowman needs to be a self-contained function (basically a file)
function snowman(u, v, x, y, w, h, handle)
    require "lib/vector"

    u = u * 2.0 - 1.0
    v = v * 2.0 - 1.0
    local ro = Vec4:new(0, 1, -2, 1)
    local center = Vec4:new(0, 0.5, 0, 1)
    local front = center:subtr(ro):nor3() -- most of the time we won't be needing the 4th component
    local right = front:cross(Vec4:new(0, 1, 0, 1)):nor3()
    local up = right:cross(front):nor3()

    local rd = right:scl(u):add(up:scl(v)):add(front:scl(1)):nor3()
    local t = 0.01

    function ground(p)
        return p.y
    end

    function ball(p, c, r)
        local dist = p:subtr(c):len3()
        return dist - r
    end

    -- Returns the object that is closest to the sample point.
    local snowman_body = Vec4:new(0, 0, 0, 1)
    local snowman_head = Vec4:new(0, 1.2, 0, 1)
    local snowman_left_eye = Vec4:new(-0.1, 1.3, -0.5, 1)
    local snowman_right_eye = Vec4:new(0.1, 1.3, -0.5, 1)
    function map(p)
--         local closest = ground(p)
        local closest = 10000.0
        local cid = 0

        local body_dist = ball(p, snowman_body, 1)
        if body_dist < closest then
            closest = body_dist
            cid = 1
        end

        local head_dist = ball(p, snowman_head, 0.5)
        if head_dist < closest then
            closest = head_dist
            cid = 2
        end

        local left_eye_dist = ball(p, snowman_left_eye, 0.05)
        if left_eye_dist < closest then
            closest = left_eye_dist
            cid = 3
        end

        local right_eye_dist = ball(p, snowman_right_eye, 0.05)
        if right_eye_dist < closest then
            closest = right_eye_dist
            cid = 3
        end

        return closest, cid
    end

    function get_color(p, dist, cid)
        if cid == 0 then
            local uu = math.floor(p.x * 3.0)
            local vv = math.floor(p.z * 3.0)
            if (uu + vv) % 2 == 0 then
                return Vec4:new(0.1, 0.1, 0.1, 1.0)
            else
                return Vec4:new(1.0, 1.0, 1.0, 1.0)
            end
        elseif cid == 1 or cid == 2 then
            return Vec4:new(1.0, 1.0, 1.0, 1.0)
        elseif cid == 3 then
            return Vec4:new(0.1, 0.1, 0.1, 1.0)
        end

        return Vec4:new(1.0, 0.0, 1.0, 1.0)
    end

    function estimate_normal(p)
        pp, _ = map(p)
        ppx, _ = map(Vec4:new(p.x - 0.01, p.y, p.z, p.w))
        ppy, _ = map(Vec4:new(p.x, p.y - 0.01, p.z, p.w))
        ppz, _ = map(Vec4:new(p.x, p.y, p.z - 0.01, p.w))

        return Vec4:new(pp - ppx, pp - ppy, pp - ppz, 0):nor3()
    end

    local ldir = Vec4:new(1, 1, -1, 0):nor3()
    local sky_dir = Vec4:new(0, 1, 0, 0)

    for i = 1, 200 do
        local p = ro:add(rd:scl(t))
        local dist, cid = map(p)

        if dist < 0.001 then
            local c = get_color(p, dist, cid)
            local nor = estimate_normal(p)

            local diffuse = math.max(ldir:dot3(nor), 0)
            local fringe = (1.0 - math.min(1.0, math.max(rd:scl(-1):dot3(nor), 0.0))) ^ 5 * 0.4
            local dome = math.max(sky_dir:dot3(nor), 0) * 0.2
            c = c:scl(clamp(diffuse + fringe + dome, 0, 1))

            set_pixel(handle, x, y, c.x, c.y, c.z, c.w)
            return
        end
        t = t + dist

        if t > 20 then
            break
        end
    end

    -- Shade sky
    local horizon = Vec4:new(0.9, 0.9, 1.0, 1.0)
    local high = Vec4:new(0.2, 0.6, 0.9, 1.0)
    local sky = horizon:mix(high, math.abs(rd.y))

    set_pixel(handle, x, y, sky.x, sky.y, sky.z, 1)
end

shade(imw, imh, im.handle, snowman)
print("Snowman done.")
