// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "material.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
};

struct Triangle {
    Vertex a, b, c;
};

/**
 * The scene representation (for now.)
 */
class Model
{
public:
    /**
     * Default constructor
     */
    Model();

    /**
     * Model can be constructed from an array of triangles.
     */
    Model(const std::vector<Triangle> &tri);

    /**
     * No copy constructor.
     */
    Model(const Model& other) = delete;

    /**
     * Destructor
     */
    ~Model();

    int id() const;

    bool load(const std::string &path, const std::string &mtl_base_dir = "");

    int get_num_verts() const;
    int get_num_tris() const;
    std::string get_load_warnings() const;
    std::string get_load_errors() const;

    const Triangle &get_triangle(int index) const;
    const std::vector<Triangle> &get_all_triangles() const;

private:
    int id_;
    bool initialized;
    std::vector<Triangle> tri;
    std::vector<Material> mat;
    std::string load_warnings, load_errors;
};

#endif // MODEL_H
