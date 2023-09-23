-- Some helper functions for camera construction.
function view_vectors(uv, ro, center)
    local front = nor3(sub3(center, ro))
    local right = nor3(cross(front, vec3(0, 1, 0)))
    local up = nor3(cross(right, front))
    local rd = nor3(add3(add3(scl3(right, uv.u), scl3(up, uv.v)), scl3(front, 2)))

    return right, up, front, rd
end
