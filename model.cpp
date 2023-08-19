// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model.h"
#include <tiny_obj_loader.h>
#include <iostream>

int model_id_counter = 0;

Model::Model() : id_(model_id_counter++), initialized(false)
{

}

Model::~Model()
{

}

int Model::id() const
{
    return id_;
}

bool Model::load(const std::string& path, const std::string &mtl_base_dir)
{
    load_warnings = "";
    load_errors = "";
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &load_warnings, &load_errors, path.c_str(), mtl_base_dir.c_str());
    if (!ret)
    {
        return false;
    }

    for (int i = 0; i < shapes.size(); i++)
    {
        int offset = 0;
        auto &mesh = shapes[i].mesh;
        for (int j = 0; j < mesh.num_face_vertices.size(); j++)
        {
            assert(mesh.num_face_vertices[j] == 3 && "Non-triangular meshes are not supported.");
            Vertex vs[3];

            for (int k = 0; k < mesh.num_face_vertices[j]; k++)
            {
                auto idx = mesh.indices[offset + k];
                Vertex v = {
                    glm::vec3(attrib.vertices[3 * idx.vertex_index + 0], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2]),
                    glm::vec3(attrib.normals[3 * idx.normal_index + 0], attrib.normals[3 * idx.normal_index + 1], attrib.normals[3 * idx.normal_index + 2]),
                    glm::vec2(0.0f)
                };
                if (idx.texcoord_index >= 0)
                {
                    v.tex_coord = glm::vec2(attrib.texcoords[2 * idx.texcoord_index + 0], attrib.texcoords[2 * idx.texcoord_index + 1]);
                }
                vs[k] = v;
            }

            tri.push_back(Triangle{ vs[0], vs[1], vs[2] });

            offset += mesh.num_face_vertices[j];
        }
    }

    initialized = true;

    return true;
}

int Model::get_num_verts() const
{
    assert(initialized && "Model is not initialized yet.");

    return tri.size() * 3;
}


int Model::get_num_tris() const
{
    assert(initialized && "Model is not initialized yet.");

    return tri.size();
}

std::string Model::get_load_warnings() const
{
    return load_warnings;
}

std::string Model::get_load_errors() const
{
    return load_errors;
}
