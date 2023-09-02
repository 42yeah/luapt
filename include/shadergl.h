// OpenGL program wrapper.
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHADERGL_H
#define SHADERGL_H

#include <glad/glad.h>
#include <string>
#include <map>
#define MAX_ERR_LOG_LENGTH 1024

/**
 * OpenGL program wrapper.
 */
class ShaderGL
{
public:
    /**
     * Default constructor
     */
    ShaderGL();

    ShaderGL(const std::string &vertex_path, const std::string &fragment_path);

    bool link_from_files(const std::string &vertex_path, const std::string &fragment_path);
    bool link_from_src(const std::string &vsrc, const std::string &fsrc);

    /**
     * No copy constructor
     */
    ShaderGL(const ShaderGL& other) = delete;

    /**
     * Destructor
     */
    ~ShaderGL();

    void use() const;

    GLint get_location(const std::string &uniform_name);

private:
    static GLuint compile(GLuint shader_type, const std::string &src);
    std::map<std::string, GLint> locations;

    bool initialized;
    GLuint program;
};

#endif // SHADERGL_H
