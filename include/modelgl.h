// ModelGL is an OpenGL wrapper over a Model. It can be directly rendered.
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODELGL_H
#define MODELGL_H

#include <glad/glad.h>
#include <iostream>
#include <memory>
#include "model.h"

/**
 * ModelGL is an OpenGL wrapper over a Model. It can be directly rendered.
 */
class ModelGL
{
public:
    /**
     * Default constructor
     */
    ModelGL();

    ModelGL(std::shared_ptr<Model> model);

    ModelGL(const ModelGL &other) = delete;

    bool import_from_model(const Model &model);
    const std::shared_ptr<Model> get_base_model() const;

    void draw() const;

    /**
     * Destructor
     */
    ~ModelGL();

private:
    bool initialized;
    GLuint vao, vbo;
    int num_verts;
    std::shared_ptr<Model> model;
};

#endif // MODELGL_H
