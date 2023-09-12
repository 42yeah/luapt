// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bbox.h"
#include <numbers>
#include <algorithm>
#include "luaenv.h"

int bvh_id_counter = 0;


BVH::BVH(std::shared_ptr<Model> model) : model(model), id_(bvh_id_counter++)
{
    // First make an empty node to fit ALL triangles inside

    BBox root_bbox = bbox();
    for (int i = 0; i < model->get_num_tris(); i++)
    {
        TriC *t = model_get_tri(model.get(), i);
        enclose(root_bbox, t->a.position);
        enclose(root_bbox, t->b.position);
        enclose(root_bbox, t->c.position);
        tri.push_back(&model->get_triangle(i));
    }

    make_node(root_bbox, 0, tri.size(), 0, 0);
}

int BVH::make_node(const BBox &bbox, int start, int size, int l, int r)
{
    Node node{ bbox, start, size, l, r };
    nodes.push_back(node);
    return nodes.size() - 1;
}

const Triangle &BVH::get_triangle(int index) const
{
    assert((index >= 0 || index < tri.size()) && "Triangle index out of bound");

    return *tri[index];
}

int BVH::id() const
{
    return id_;
}

const Node &BVH::get_node(int index) const
{
    assert((index >= 0 || index < nodes.size()) && "Node index out of bound");
    return nodes[index];
}

void BVH::set_children(int who, int l, int r)
{
    assert((who >= 0 || who < nodes.size()) && "Node index out of bound");
    nodes[who].l = l;
    nodes[who].r = r;
}

int BVH::get_num_triangles() const
{
    return tri.size();
}

int BVH::get_num_nodes() const
{
    return nodes.size();
}

int BVH::partition(bool *pred, int begin, int end)
{
    assert(begin >= 0 && end < tri.size() && "Invalid partition range");

    while (begin <= end)
    {
        if (!pred[begin])
        {
            const Triangle *t = tri[begin];
            tri[begin] = tri[end];
            tri[end] = t;

            // Gotta swap that pred as well
            bool p = pred[begin];
            pred[begin] = pred[end];
            pred[end] = p;

            end--;
        }
        else
        {
            begin++;
        }
    }
    return begin;
}
