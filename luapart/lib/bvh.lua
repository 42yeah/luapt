-- In-house BVH implementation.
require "lib/pervasives"

function determine_side(p, offset, axis)
    if axis == 0 then
        return p.x < offset.x
    elseif axis == 1 then
        return p.y < offset.y
    else
        return p.z < offset.z
    end
end

function determine_area_ratio(poff, span, axis)
    if axis == 0 then
        return poff.x / span.x, (span.x - poff.x) / span.x
    elseif axis == 1 then
        return poff.y / span.y, (span.y - poff.y) / span.y
    else
        return poff.z / span.z, (span.z - poff.z) / span.z
    end
end

function bvh_construct(bvh, node_idx, start, fin)
    if fin - start + 1 <= 8 then
        -- No need to make BVH; not a lot of triangles here
        return
    end

    local n = bvh_get_node(bvh, node_idx)
    local span = sub3(n.bbox.max, n.bbox.min)

    -- split them into 8 buckets
    local num_buckets = 8
    local split_step = scl3(span, 1 / num_buckets)

    -- Bests
    local best_axis = 0
    local best_step = 1
    local best_sah = 1e309
    local best_offset = 0

    for axis = 0, 2 do
        -- Tentatively partition them along these axis

        for step = 1, (num_buckets - 1) do
            -- 1. Somehow partition it.
            local table = make_partitioning_table(bvh)
            local poff = scl3(split_step, step) -- Plane offset
            local plane = add3(n.bbox.min, poff)
            for i = start, fin do
                local tri = bvh_get_tri(bvh, i)
                local centroid = scl3(add3(add3(tri.a.position, tri.b.position), tri.c.position), 1 / 3)

                table[i] = determine_side(centroid, plane, axis)
            end
            local offset = partition(bvh, table, start, fin)

            -- 2. Calculate SAH. Record the best one.
            local left, right = determine_area_ratio(poff, span, axis)
            local sah = 1 + 2 * left * ((offset - start) / (fin - start + 1)) ^ 2 + 2 * right * ((fin - offset + 1) / (fin - start + 1)) ^ 2

            if sah < best_sah then
                best_sah = sah
                best_step = step
                best_axis = axis
                best_offset = offset
            end
        end
    end

    -- That's not very constructive.
    if best_offset == start or best_offset == fin + 1 then
        return
    end

    -- Partition using the best one.
    local table = make_partitioning_table(bvh)
    local poff = scl3(split_step, best_step)
    local plane = add3(n.bbox.min, poff)
    for i = start, fin do
        local tri = bvh_get_tri(bvh, i)
        local centroid = scl3(add3(add3(tri.a.position, tri.b.position), tri.c.position), 1 / 3)

        table[i] = determine_side(centroid, plane, best_axis)
    end
    local offset = partition(bvh, table, start, fin)

    local left_box = bbox()
    local right_box = bbox()
    for i = start, offset - 1 do
        local tri = bvh_get_tri(bvh, i)
        enclose(left_box, tri.a.position)
        enclose(left_box, tri.b.position)
        enclose(left_box, tri.c.position)
    end
    for i = offset, fin do
        local tri = bvh_get_tri(bvh, i)
        enclose(right_box, tri.a.position)
        enclose(right_box, tri.b.position)
        enclose(right_box, tri.c.position)
    end

    local l = bvh_push_node(bvh, left_box, start, offset - start, 0, 0)
    local r = bvh_push_node(bvh, right_box, offset, fin - offset + 1, 0, 0)
    bvh_node_set_children(bvh, node_idx, l, r)

    -- Recurse into l and r ???
    bvh_construct(bvh, l, start, offset - 1)
    bvh_construct(bvh, r, offset, fin)
end

function bvh_hits(bvh, node_idx, ro, rd)
    local n = bvh_get_node(bvh, node_idx)
    if intersect_box(ro, rd, n.bbox) == nil then
        return nil
    end

    local closest_tri = nil
    local closest_uvt = nil

    -- If hits, and is leave node, then we iterate over the triangles
    -- Otherwise, we recurse into the leave node
    if n.l == 0 and n.r == 0 then
        for i = n.start, n.start + n.size - 1 do
            local tri = bvh_get_tri(bvh, i)
            local uvt = intersect(ro, rd, tri, 0.01, 100.0)

            if uvt ~= nil then
                if closest_uvt == nil or uvt.z < closest_uvt.z then
                    closest_tri = tri
                    closest_uvt = uvt
                end
            end
        end
    else
        local tri_l, uvt_l = bvh_hits(bvh, n.l, ro, rd)
        local tri_r, uvt_r = bvh_hits(bvh, n.r, ro, rd)
        if uvt_l == nil then
            return tri_r, uvt_r
        elseif uvt_r == nil then
            return tri_l, uvt_l
        elseif uvt_l.z < uvt_r.z then
            return tri_l, uvt_l
        end
        return tri_r, uvt_r
    end

    return closest_tri, closest_uvt
end
