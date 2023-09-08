require "lib/vector"
local pprint = require "lib/pprint"

-- Check if a ray situated at ro with direction rd intersects with triangle tri.
-- The brute force method
function intersect_bf(ro, rd, tri, tmin, tmax)
    -- Calculate plane N^T x = c
    local c = dot3(tri.a.position, tri.a.normal)
    -- <N^T, d>
    -- if the value is less than some threshold, we assume rd is parallel to N and therefore won't have an intersection
    local ntd = dot3(tri.a.normal, rd)
    if math.abs(ntd) < 0.001 then
        return nil
    end

    local t = (c - dot3(tri.a.normal, ro)) / ntd
    if t < tmin or t > tmax then
        return nil
    end

    local p = add3(ro, scl3(rd, t))

    -- If there's an intersection, try to figure out the barycentric coordinate to check whether it's in the triangle or not
    local ab = sub3(tri.b.position, tri.a.position)
    local ac = sub3(tri.c.position, tri.a.position)
    local sabc = len3(cross(ab, ac)) / 2

    local ap = sub3(p, tri.a.position)
    local cb = sub3(tri.b.position, tri.c.position)
    local cp = sub3(p, tri.c.position)
    local u = len3(cross(ab, ap)) / 2 / sabc
    local v = len3(cross(ap, ac)) / 2 / sabc
    local w = len3(cross(cp, cb)) / 2 / sabc

    if u < 0 or v < 0 or w < 0 or u > 1 or v > 1 or w > 1 or math.abs(u + v + w - 1) > 0.01 then
        return nil
    end

    return vec3(u, v, t)
end

-- The Moller-Trumbore method
function intersect_mt(ro, rd, tri, tmin, tmax)
    local e1 = sub3(tri.b.position, tri.a.position)
    local e2 = sub3(tri.c.position, tri.a.position)

    local pvec = cross(rd, e2)
    local det = dot3(e1, pvec)

    -- Are they almost parallel?
    if (math.abs(det) < 0.0001) then
        return nil
    end

    local inv = 1.0 / det
    local tvec = sub3(ro, tri.a.position)
    local u = dot3(pvec, tvec) * inv
    if u < 0 or u > 1 then
        return nil
    end

    local qvec = cross(tvec, e1)
    local v = dot3(rd, qvec) * inv

    if v < 0 or u + v > 1 then
        return nil
    end

    local t = dot3(e2, qvec) * inv
    if t < tmin or t > tmax then
        return nil
    end

    return vec3(u, v, t)
end

function intersect_box(ro, rd, box)
    return nil
end

intersect = intersect_mt
