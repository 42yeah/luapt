// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modelgl.h"

ModelGL::ModelGL() : initialized(false), vao(GL_NONE), vbo(GL_NONE), num_verts(0), model(nullptr)
{

}

ModelGL::ModelGL(std::shared_ptr<Model> model) : ModelGL()
{
    if (model != nullptr)
    {
        this->model = model;
        this->num_verts = model->get_num_verts();
        import_from_model(*model);
    }
}


bool ModelGL::import_from_model(const Model &model)
{
    if (initialized)
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    initialized = false;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const std::vector<Triangle> &tris = model.get_all_triangles();
    glBufferData(GL_ARRAY_BUFFER, tris.size() * sizeof(Triangle), tris.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (sizeof(glm::vec3)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (sizeof(glm::vec3) * 2));

    initialized = true;
    return true;
}

const std::shared_ptr<Model> ModelGL::get_base_model() const
{
    assert(initialized && "ModelGL is not initialized");
    return model;
}

ModelGL::~ModelGL()
{
    if (initialized)
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        initialized = false;
    }
}

void ModelGL::draw() const
{
    assert(initialized && "ModelGL is not initialized");
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, num_verts);
}

