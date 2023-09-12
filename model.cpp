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

Model::Model(const std::vector<Triangle> &tri) : Model()
{
    if (tri.size() > 0)
    {
        this->tri = tri;
        initialized = true;
    }
}

Model::~Model()
{
    initialized = false;
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
            int material_id = mesh.material_ids[j];

            for (int k = 0; k < mesh.num_face_vertices[j]; k++)
            {
                auto idx = mesh.indices[offset + k];

                Vertex v = { glm::vec3(0.0f), glm::vec3(0.0f), glm::vec2(0.0f), material_id };
                if (idx.vertex_index != -1)
                {
                    v.position = glm::vec3(attrib.vertices[3 * idx.vertex_index + 0], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2]);
                }
                if (idx.normal_index != -1)
                {
                    v.normal = glm::vec3(attrib.normals[3 * idx.normal_index + 0], attrib.normals[3 * idx.normal_index + 1], attrib.normals[3 * idx.normal_index + 2]);
                }
                if (idx.texcoord_index != -1)
                {
                    v.tex_coord = glm::vec2(attrib.texcoords[2 * idx.texcoord_index + 0], attrib.texcoords[2 * idx.texcoord_index + 1]);
                }

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

    for (int i = 0; i < materials.size(); i++)
    {
        Material m(materials[i]);
        mat.push_back(m);
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

const Triangle &Model::get_triangle(int index) const
{
    assert(initialized && "Model is not initialized yet.");
    assert((index >= 0 || index < tri.size()) && "Triangle index out of bound");

    return tri[index];
}

const std::vector<Triangle> &Model::get_all_triangles() const
{
    assert(initialized && "Model is not initialized yet.");

    return tri;
}

HitInfo Model::get_hit_info(int material_id, const glm::vec2 &uv) const
{
    assert(material_id >= 0 && material_id < mat.size() && "Material ID out of bounds");
    return mat[material_id].get_hit_info(uv);
}
