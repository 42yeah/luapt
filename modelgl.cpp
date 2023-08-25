// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modelgl.h"

ModelGL::ModelGL() : initialized(false), vao(GL_NONE), vbo(GL_NONE), model(nullptr)
{

}

ModelGL::ModelGL(const Model &model) : ModelGL()
{
    import_from_model(model);
}


bool ModelGL::import_from_model(const Model &model)
{
    if (initialized)
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    initialized = false;
    this->model = &model;

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

const Model *ModelGL::get_base_model() const
{
    return model;
}

ModelGL::~ModelGL()
{

}
