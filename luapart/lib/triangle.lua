require "lib/vector"
local pprint = require "lib/pprint"

Vertex = {
    position = Vec4:new(0, 0, 0, 1),
    normal = Vec4:new(0, 0, 0, 1),
    tex_coord = Vec4:new(0, 0, 0, 1)
}

function Vertex:new()
    local ret = {}
    setmetatable(ret, self)
    self.__index = self
    ret.position = Vec4:new(0, 0, 0, 1)
    ret.normal = Vec4:new(0, 0, 0, 1)
    ret.tex_coord = Vec4:new(0, 0, 0, 1)
    return ret
end

function Vertex:set(p, n, t)
    self.position = p
    self.normal = n
    self.tex_coord = t
end

Triangle = {
    a = Vertex:new(),
    b = Vertex:new(),
    c = Vertex:new()
}

function Triangle:new()
    local ret = {}
    setmetatable(ret, self)
    self.__index = self
    ret.a = Vertex:new()
    ret.b = Vertex:new()
    ret.c = Vertex:new()
    return ret
end

function Triangle:set(a, b, c)
    self.a = a
    self.b = b
    self.c = c
end

-- Check if a ray situated at ro with direction rd intersects with triangle tri.
function intersect(ro, rd, tri, tmin, tmax)
    -- Calculate plane N^T x = c
    local c = tri.a.position:dot3(tri.a.normal)
    -- <N^T, d>
    -- if the value is less than some threshold, we assume rd is parallel to N and therefore won't have an intersection
    local ntd = tri.a.normal:dot3(rd)
    if math.abs(ntd) < 0.001 then
        return nil
    end

    local t = (c - tri.a.normal:dot3(ro)) / ntd
    if t < tmin or t > tmax then
        return nil
    end

    local p = ro:add(rd:scl(t))

    -- If there's an intersection, try to figure out the barycentric coordinate to check whether it's in the triangle or not
    local ab = tri.b.position:subtr(tri.a.position)
    local ac = tri.c.position:subtr(tri.a.position)
    local sabc = ab:cross(ac):len3() / 2

    local ap = p:subtr(tri.a.position)
    local cb = tri.b.position:subtr(tri.c.position)
    local cp = p:subtr(tri.c.position)
    local u = (ab:cross(ap):len3() / 2) / sabc
    local v = (ap:cross(ac):len3() / 2) / sabc
    local w = (cp:cross(cb):len3() / 2) / sabc

    if u < 0 or v < 0 or w < 0 or u > 1 or v > 1 or w > 1 or math.abs(u + v + w - 1) > 0.01 then
        return nil
    end

    return t
end
