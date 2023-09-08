// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BBOX_H
#define BBOX_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "model.h"

/**
 * A simple bounding box.
 */
struct BBox
{
    glm::vec3 min, max;

    BBox();

    void enclose(const glm::vec3 &p);

    // We won't define the intersection function here - it's nontrivial.
};

struct Node
{
    BBox bbox;
    size_t start, size;
    size_t l, r; // Left child & right child
};

/**
 * A Bounding Volume Hierarchy (BVH), used to accelerate pathtracing.
 */
class BVH
{
public:
    BVH(std::shared_ptr<Model> model);

    int id() const;

    size_t make_node(const BBox &bbox, size_t start, size_t size, size_t l, size_t r);
    Node &get_node(int index);

    const Triangle &get_triangle(int index) const;

private:
    std::vector<const Triangle *> tri; // We will need to rearrange these triangles
    std::vector<Node> nodes;
    std::shared_ptr<Model> model;

    int id_;
};

#endif // BBOX_H
