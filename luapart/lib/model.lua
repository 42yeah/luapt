require "lib/vector"
require "lib/triangle"

Model = {
    handle = 0,
    path = "",
    __gc = function()
        free_model(self.handle)
    end
}

function Model:new(path)
    local ret = {}
    setmetatable(ret, self)
    self.__index = self
    ret.path = path
    ret.handle = make_model(path)
    return ret
end

function Model:tri_count()
    return model_tri_count(self.handle)
end

function Model:tri(index)
    local apx, apy, apz, anx, any, anz, au, av, bpx, bpy, bpz, bnx, bny, bnz, bu, bv, cpx, cpy, cpz, cnx, cny, cnz, cu, cv = model_get_tri(self.handle, index - 1)

    local tri = Triangle:new()
    tri.a:set(Vec4:new(apx, apy, apz, 1.0), Vec4:new(anx, any, anz, 1.0), Vec4:new(au, av))
    tri.b:set(Vec4:new(bpx, bpy, bpz, 1.0), Vec4:new(bnx, bny, bnz, 1.0), Vec4:new(bu, bv))
    tri.c:set(Vec4:new(cpx, cpy, cpz, 1.0), Vec4:new(cnx, cny, cnz, 1.0), Vec4:new(cu, cv))

    return tri
end
