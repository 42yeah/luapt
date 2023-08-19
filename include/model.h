// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
};

struct Triangle {
    Vertex a, b, c;
};

/**
 * @todo write docs
 */
class Model
{
public:
    /**
     * Default constructor
     */
    Model();

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

private:
    int id_;
    bool initialized;
    std::vector<Triangle> tri;
    std::string load_warnings, load_errors;
};

#endif // MODEL_H
