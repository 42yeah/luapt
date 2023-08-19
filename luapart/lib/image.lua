require "lib/vector"

Image = {
    handle = 0,
    w = 0,
    h = 0,
    __gc = function()
        if w ~= 0 and h ~= 0 then
            free_image(self.handle)
        end
    end
}

function Image:new(w, h)
    local ret = {}
    setmetatable(ret, self)
    self.__index = self
    ret.handle = make_image(w, h)
    ret.w = w
    ret.h = h
    return ret
end

function Image:save(name)
    save_image(self.handle, name)
end

function Image:pixel(x, y, r, g, b)
    set_pixel(self.handle, x, y, clamp(r, 0, 1), clamp(g, 0, 1), clamp(b, 0, 1))
end

function Image:pixel_vec4(x, y, vec4)
    set_pixel(self.handle, x, y, clamp(vec4.x, 0, 1), clamp(vec4.y, 0, 1), clamp(vec4.z, 0, 1))
end
