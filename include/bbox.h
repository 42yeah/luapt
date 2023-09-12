// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BBOX_H
#define BBOX_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "model.h"
#include "luamath.h"

/**
 * A Bounding Volume Hierarchy (BVH), used to accelerate pathtracing.
 */
class BVH
{
public:
    BVH(std::shared_ptr<Model> model);

    int id() const;

    int make_node(const BBox &bbox, int start, int size, int l, int r);
    const Node &get_node(int index) const;
    void set_children(int who, int l, int r);
    int get_num_nodes() const;

    const Triangle &get_triangle(int index) const;
    int get_num_triangles() const;

    /**
     * Partition the table based on preds.
     * If it is true, then it comes to the left side.
     * Otherwise it goes to the right side.
     */
    int partition(bool *pred, int begin, int end);

private:
    std::vector<const Triangle *> tri; // We will need to rearrange these triangles
    std::vector<Node> nodes;
    std::shared_ptr<Model> model;

    int id_;
};

#endif // BBOX_H
