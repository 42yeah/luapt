// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shadergl.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cassert>


ShaderGL::ShaderGL() : initialized(false), program(GL_NONE)
{

}

ShaderGL::~ShaderGL()
{
    if (initialized)
    {
        glDeleteProgram(program);
        initialized = false;
    }
}

ShaderGL::ShaderGL(const std::string &vertex_path, const std::string &fragment_path) : ShaderGL()
{
    link_from_files(vertex_path, fragment_path);
}

bool ShaderGL::link_from_files(const std::string &vertex_path, const std::string &fragment_path)
{
    initialized = false;

    std::ifstream vreader(vertex_path);
    if (!vreader.good())
    {
        std::cerr << "WARNING: cannot open: " << vertex_path << std::endl;
        return false;
    }
    std::stringstream ss;
    ss << vreader.rdbuf();
    std::string vsrc = ss.str();
    vreader.close();

    ss.str("");
    std::ifstream freader(fragment_path);
    if (!freader.good())
    {
        std::cerr << "WARNING: cannot open: " << fragment_path << std::endl;
        return false;
    }
    ss << freader.rdbuf();
    std::string fsrc = ss.str();
    freader.close();

    return link_from_src(vsrc, fsrc);
}

bool ShaderGL::link_from_src(const std::string &vsrc, const std::string &fsrc)
{
    initialized = false;

    GLuint vs = compile(GL_VERTEX_SHADER, vsrc),
        fs = compile(GL_FRAGMENT_SHADER, fsrc);

    if (vs == GL_NONE || fs == GL_NONE)
    {
        if (vs != GL_NONE)
        {
            glDeleteShader(vs);
        }
        if (fs != GL_NONE)
        {
            glDeleteShader(fs);
        }
        return false;
    }

    if (program != GL_NONE)
    {
        glDeleteProgram(program);
    }
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    // The rest is reporting if something's wrong with the program
    char log[MAX_ERR_LOG_LENGTH] = { 0 };
    glGetProgramInfoLog(program, sizeof(log), nullptr, log);

    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        std::cerr << "WARNING: Cannot link: " << log << std::endl;

        // Cleanup
        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    if (strnlen(log, sizeof(log)) != 0)
    {
        std::cerr << "WARNING: Program link warning: " << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    initialized = true;

    return true;
}

GLuint ShaderGL::compile(GLuint shader_type, const std::string &src)
{
    GLuint shader = glCreateShader(shader_type);
    const char *raw = src.c_str();
    glShaderSource(shader, 1, &raw, nullptr);
    glCompileShader(shader);

    char log[MAX_ERR_LOG_LENGTH] = { 0 };
    glGetShaderInfoLog(shader, sizeof(log), nullptr, log);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        std::cerr << "WARNING: Cannot compile: " << log << std::endl;

        // Cleanup
        glDeleteShader(shader);
        return GL_NONE;
    }

    if (strnlen(log, sizeof(log)) != 0)
    {
        std::cerr << "WARNING: Shader compile warning: " << log << std::endl;
    }

    return shader;
}

void ShaderGL::use() const
{
    assert(initialized && "ShaderGL is not initialized");
    glUseProgram(program);
}
