require "math"

Vec4 = {
    x = 0,
    y = 0,
    z = 0,
    w = 1
}

function Vec4:make()
    local ret = {}
    setmetatable(ret, self)
    self.__index = self
    ret.x = 0
    ret.y = 0
    ret.z = 0
    ret.w = 1
    return ret
end

function Vec4:new(x, y, z, w)
    local ret = Vec4:make()
    ret.x = x
    ret.y = y
    ret.z = z
    ret.w = w
    return ret
end

function Vec4:add(other)
    return Vec4:new(self.x + other.x, self.y + other.y, self.z + other.z, self.w + other.w)
end

function Vec4:subtr(other)
    return Vec4:new(self.x - other.x, self.y - other.y, self.z - other.z, self.w - other.w)
end

function Vec4:dot(other)
    return self.x * other.x + self.y * other.y + self.z * other.z + self.w * other.w
end

function Vec4:dot3(other)
    return self.x * other.x + self.y * other.y + self.z * other.z
end

function Vec4:mul(other)
    return Vec4:new(self.x * other.x, self.y * other.y, self.z * other.z, self.w * other.w)
end

function Vec4:scl(s)
    return Vec4:new(s * self.x, s * self.y, s * self.z, s * self.w)
end

-- Crossing a function only takes the first
function Vec4:cross(other)
    return Vec4:new(self.y * other.z - self.z * other.y, self.z * other.x - self.x * other.z, self.x * other.y - self.y * other.x, 1.0)
end

function Vec4:len3()
    return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z)
end

function Vec4:len4()
    return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w)
end

function Vec4:nor3()
    l3 = self:len3()
    return Vec4:new(self.x / l3, self.y / l3, self.z / l3, 1)
end

function Vec4:nor4()
    l4 = self:len4()
    return self:scl(1 / l4)
end

function clamp(x, a, b)
    if x < a then
        return a
    elseif x > b then
        return b
    end
    return x
end
