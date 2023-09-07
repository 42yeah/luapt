-- Bounding box construction and intersection test.

BBox = {

}

function BBox:new()
    ret = {}
    setmetatable(ret, self)
    ret.__index = self
    return ret
end
