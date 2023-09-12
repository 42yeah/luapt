// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "material.h"

int material_id_counter = 0;

Material::Material() : material_name("boring"), metallic(0), metallic_tex(nullptr), ior(1.0f), emission(RGB<float>(0.0f, 0.0f, 0.0f)), emissive_tex(nullptr), ambient(RGB<float>(0.1f, 0.0f, 0.1f)), ambient_tex(nullptr), diffuse({ 1.0f, 0.0f, 1.0f }), diffuse_tex(nullptr), normal_tex(nullptr), specular(RGB<float>(0.0f, 0.0f, 0.0f)), specular_tex(nullptr), id_(material_id_counter++)
{

}

Material::Material(const tinyobj::material_t &mat) : Material()
{
    material_name = mat.name;
    metallic = mat.metallic;
    if (!mat.metallic_texname.empty())
    {
        metallic_tex = std::make_shared<FloatImage>();
        metallic_tex->load(mat.metallic_texname); // TODO: issues might arise in the future (path)
    }
    ior = mat.ior;
    emission = RGB<float>(mat.emission[0], mat.emission[1], mat.emission[2]);
    if (!mat.emissive_texname.empty())
    {
        emissive_tex = std::make_shared<FloatImage>();
        emissive_tex->load(mat.emissive_texname);
    }
    ambient = RGB<float>(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
    if (!mat.ambient_texname.empty())
    {
        ambient_tex = std::make_shared<FloatImage>();
        ambient_tex->load(mat.ambient_texname);
    }
    diffuse = RGB<float>(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
    if (!mat.diffuse_texname.empty())
    {
        diffuse_tex = std::make_shared<FloatImage>();
        diffuse_tex->load(mat.diffuse_texname);
    }
    if (!mat.normal_texname.empty())
    {
        normal_tex = std::make_shared<FloatImage>();
        normal_tex->load(mat.normal_texname);
    }
    specular = RGB<float>(mat.specular[0], mat.specular[1], mat.specular[2]);
    if (!mat.specular_texname.empty())
    {
        specular_tex = std::make_shared<FloatImage>();
        specular_tex->load(mat.specular_texname);
    }
}

Material::~Material()
{

}

const std::string &Material::get_name() const
{
    return material_name;
}

float Material::get_metallic(const glm::vec2 &uv) const
{
    if (metallic_tex)
    {
        return metallic_tex->sample_rgb(uv.x, uv.y, SampleMethod::Repeat).r;
    }
    return metallic;
}

float Material::get_ior() const
{
    return ior;
}

RGB<float> Material::get_emission(const glm::vec2 &uv) const
{
    if (emissive_tex)
    {
        return emissive_tex->sample_rgb(uv.x, uv.y, SampleMethod::Repeat);
    }
    return emission;
}

RGB<float> Material::get_ambient(const glm::vec2 &uv) const
{
    if (ambient_tex)
    {
        return ambient_tex->sample_rgb(uv.x, uv.y, SampleMethod::Repeat);
    }
    return ambient;
}

RGB<float> Material::get_diffuse(const glm::vec2 &uv) const
{
    if (diffuse_tex)
    {
        return diffuse_tex->sample_rgb(uv.x, uv.y, SampleMethod::Repeat);
    }
    return diffuse;
}

RGB<float> Material::get_normal_bump(const glm::vec2 &uv) const
{
    if (normal_tex)
    {
        return normal_tex->sample_rgb(uv.x, uv.y, SampleMethod::Repeat);
    }
    // Return straight up (no offset)
    return RGB<float>(0.0f, 1.0f, 0.0f);
}

RGB<float> Material::get_specular(const glm::vec2 &uv) const
{
    if (specular_tex)
    {
        return specular_tex->sample_rgb(uv.x, uv.y, SampleMethod::Repeat);
    }
    return specular;
}


int Material::id() const
{
    return id_;
}
