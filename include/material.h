// Material describes what the "material" is. It encloses textures and other things as well.
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>
#include "image.h"

/**
 * Material describes what the "material" is. It encloses textures and other things as well.
 * Material is heavy and therefore must be specified by a flyweight by the others.
 */
class Material
{
public:
    /**
     * Default constructor
     */
    Material();

    /**
     * Support for direct importation from tinyobj
     */
    Material(const tinyobj::material_t &mat);

    /**
     * Copy constructor
     */
    Material(const Material& other) = delete;

    /**
     * Destructor
     */
    ~Material();

    const std::string &get_name() const;
    float get_metallic(const glm::vec2 &uv) const;
    float get_ior() const;
    RGB<float> get_emission(const glm::vec2 &uv) const;
    RGB<float> get_ambient(const glm::vec2 &uv) const;
    RGB<float> get_diffuse(const glm::vec2 &uv) const;
    RGB<float> get_normal_bump(const glm::vec2 &uv) const;
    RGB<float> get_specular(const glm::vec2 &uv) const;

    int id() const;

private:
    std::string material_name;

    float metallic;
    std::shared_ptr<FloatImage> metallic_tex;

    float ior;
    RGB<float> emission;
    std::shared_ptr<FloatImage> emissive_tex;
    RGB<float> ambient;
    std::shared_ptr<FloatImage> ambient_tex;
    RGB<float> diffuse;
    std::shared_ptr<FloatImage> diffuse_tex;
    std::shared_ptr<FloatImage> normal_tex;
    RGB<float> specular;
    std::shared_ptr<FloatImage> specular_tex;

    int id_;
};

#endif // MATERIAL_H
