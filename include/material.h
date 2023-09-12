// Material describes what the "material" is. It encloses textures and other things as well.
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>
#include "image.h"
#include "luamath.h"

/**
 * HitInfo describes the hit surface.
 * We can't have high-level definition here, it has to be extern C -
 * this looks ugly but I suppose there's nothing we can do. I don't want that many copies
 */
extern "C"
{
    struct HitInfo
    {
        float metallic;
        float ior;
        Vec3C emission;
        Vec3C ambient;
        Vec3C diffuse;
        Vec3C normal_bump;
        Vec3C specular;
    };
}


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
    Material(const Material& other) = default;

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

    HitInfo get_hit_info(const glm::vec2 &uv) const;

    int id() const;

private:
    std::string material_name;

    float metallic;
    std::shared_ptr<U8Image> metallic_tex;

    float ior;
    RGB<float> emission;
    std::shared_ptr<U8Image> emissive_tex;
    RGB<float> ambient;
    std::shared_ptr<U8Image> ambient_tex;
    RGB<float> diffuse;
    std::shared_ptr<U8Image> diffuse_tex;
    std::shared_ptr<U8Image> normal_tex;
    RGB<float> specular;
    std::shared_ptr<U8Image> specular_tex;

    int id_;
};


#endif // MATERIAL_H
