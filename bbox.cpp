// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bbox.h"
#include <numbers>

int bvh_id_counter = 0;

BBox::BBox() : min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest())
{

}

void BBox::enclose(const glm::vec3 &p)
{
    min = glm::min(min, p);
    max = glm::max(max, p);
}

BVH::BVH(std::shared_ptr<Model> model) : model(model), id_(bvh_id_counter++)
{
    // First make an empty node to fit ALL triangles inside

    BBox root_bbox;
    for (const auto &t : model->get_all_triangles())
    {
        root_bbox.enclose(t.a.position);
        root_bbox.enclose(t.b.position);
        root_bbox.enclose(t.c.position);
        tri.push_back(&t);
    }
    make_node(root_bbox, 0, tri.size(), 0, 0);
}

size_t BVH::make_node(const BBox &bbox, size_t start, size_t size, size_t l, size_t r)
{
    Node n{ bbox, start, size, l, r };
    nodes.push_back(n);
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

Node &BVH::get_node(int index)
{
    assert((index >= 0 || index < nodes.size()) && "Node index out of bound");
    return nodes[index];
}
