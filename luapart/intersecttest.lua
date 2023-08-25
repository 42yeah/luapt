require "lib/model"
require "lib/triangle"
local pprint = require "lib/pprint"

a = Vertex:new()
a:set(Vec4:new(0, 0, 0, 1), Vec4:new(0, 0, -1, 1), Vec4:new(0, 0, 0, 1))
b = Vertex:new()
b:set(Vec4:new(1, 0, 0, 1), Vec4:new(0, 0, -1, 1), Vec4:new(0, 0, 0, 1))
c = Vertex:new()
c:set(Vec4:new(0, 1, 0, 1), Vec4:new(0, 0, -1, 1), Vec4:new(0, 0, 0, 1))

local tri = Triangle:new()
tri:set(a, b, c)

local ro = Vec4:new(-0.5, 0.4, -2, 1)
local rd = Vec4:new(0, 0, 1, 1):nor3()

pprint(intersect(ro, rd, tri, 0.01, 100.0))
