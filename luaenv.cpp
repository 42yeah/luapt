// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "luaenv.h"
#include <cassert>
#include <sstream>
#include <algorithm>
#include <random>
#include <image.h>


Lua::Lua() : lua_ready(false), l(nullptr)
{
    l = luaL_newstate();
    luaL_openlibs(l);

    lua_pushlightuserdata(l, this);
    lua_setglobal(l, "luaenv");

    lua_register(l, "whatever", Lua::whatever);

    if (l)
    {
        lua_ready = true;
    }
}

int Lua::execute(const std::string &buffer)
{
    assert(lua_ready && l && "Lua is not ready");

    int error = luaL_loadbuffer(l, buffer.c_str(), buffer.size(), "line") ||
        lua_pcall(l, 0, 0, 0);

    if (error)
    {
        std::stringstream ss;
        ss << lua_tostring(l, -1);
        res()->report_error(ss.str());
        lua_pop(l, 1);
    }

    return error;
}

int Lua::execute_file(const std::string &file)
{
    assert(lua_ready && l && "Lua is not ready");

    std::ifstream reader(file);
    if (!reader.good())
    {
        std::stringstream ss;
        ss << "Cannot open: " << file;
        res()->report_error(ss.str());
        return 1;
    }

    std::stringstream ss;
    ss << reader.rdbuf();
    return execute(ss.str());
}

Lua::~Lua()
{
    if (lua_ready && l)
    {
        lua_close(l);
    }
}

void generate_demo_image(int w, int h, const char *path)
{
    std::shared_ptr<U8Image> img = generate_gradient_image(w, h);
    img->save(path);
}


void Lua::call_shade(const std::string &src, float u, float v, int x, int y, int w, int h)
{
    assert(lua_ready && l && "Lua is not ready");

    int error = luaL_loadbuffer(l, src.c_str(), src.size(), "shade");
    if (error)
    {
        std::stringstream err;
        err << "Failed to load source: " << lua_tostring(l, -1);
        res()->report_error(err.str());
    }

    lua_createtable(l, 0, 6);

    lua_pushnumber(l, u);
    lua_setfield(l, -2, "u");

    lua_pushnumber(l, v);
    lua_setfield(l, -2, "v");

    lua_pushnumber(l, x);
    lua_setfield(l, -2, "x");

    lua_pushnumber(l, y);
    lua_setfield(l, -2, "y");

    lua_pushnumber(l, w);
    lua_setfield(l, -2, "w");

    lua_pushnumber(l, h);
    lua_setfield(l, -2, "h");

    lua_setglobal(l, "pparams");

    if (lua_pcall(l, 0, 0, 0))
    {
        std::stringstream err;
        err << "Failed to execute parallel script: " << lua_tostring(l, -1);
        res()->report_error(err.str());
    }
}

Lua *Lua::get_self(lua_State *l)
{
    lua_getglobal(l, "luaenv");
    Lua *self = (Lua *) (lua_touserdata(l, -1));
    assert(self != nullptr && "luaenv not found");
    lua_pop(l, 1);
    return self;
}

int Lua::whatever(lua_State *l)
{
    Lua *lua = get_self(l);
    lua_pushnumber(l, lua->distrib(lua->dev));
    return 1;
}


// Images
FloatImage *make_image(int width, int height)
{
    Resources *r = res();
    std::shared_ptr<FloatImage> img = std::make_shared<FloatImage>(width, height, 4);
    r->images.push_back(img);
    return img.get();
}

FloatImage *load_image(const char *path)
{
    Resources *r = res();
    std::shared_ptr<FloatImage> img = std::make_shared<FloatImage>(0, 0, 3);
    if (!img->load(path))
    {
        std::stringstream ss;
        ss << "Cannot load image: " << path << " (returning nil)";
        r->report_error(ss.str());
        return nullptr;
    }
    r->images.push_back(img);
    return img.get();
}

void set_pixel(FloatImage *img, int x, int y, float r, float g, float b)
{
    img->set_rgb(x, y, RGB<float>(r, g, b));
}

bool save_image(FloatImage *img, const char *path)
{
    return img->save(path);
}

void free_image(FloatImage *img)
{
    Resources *r = res();
    auto it = std::find_if(r->images.begin(), r->images.end(), [&](const std::shared_ptr<BaseImage> im)
    {
        return im.get() == img;
    });
    assert(it != r->images.end() && "Non-existent image");
    r->images.erase(it, it + 1); // WARNING: img now becomes a dangling pointer
}

Vec3C get_pixel(FloatImage *img, int x, int y)
{
    RGB<float> rgb = img->get_rgb_float(x, y);
    return { rgb.r, rgb.g, rgb.b };
}

Vec3C sample_image(FloatImage *img, float u, float v)
{
    // One of these days...
    RGB<float> rgb = img->sample_rgb(u, v, SampleMethod::Repeat);
    return { rgb.r, rgb.g, rgb.b };
}


// Models
Model *make_model(const char *path, const char *mtl_base_path)
{
    std::shared_ptr<Model> model = std::make_shared<Model>();
    std::string base_path = mtl_base_path ? mtl_base_path : "";
    if (!model->load(path, base_path))
    {
        return nullptr;
    }

    // TODO: warnings here

    Resources *r = res();
    r->models.push_back(model);
    return model.get();
}

int model_tri_count(const Model *model)
{
    return model->get_num_tris();
}

TriC *model_get_tri(const Model *model, int index)
{
    assert(index >= 0 && index < model->get_num_tris() && "Model index out of bounds");

    // Don't worry honey, it's perfectly fine... maybe.
    return (TriC *) &model->get_triangle(index);
}

void free_model(Model *model)
{
    Resources *r = res();
    auto it = std::find_if(r->models.begin(), r->models.end(), [&](const std::shared_ptr<Model> m)
    {
        return m.get() == model;
    });
    assert(it != r->models.end() && "Non-existent model");
    r->models.erase(it, it + 1); // WARNING: img now becomes a dangling pointer
}

BVH *make_bvh(Model *model)
{
    Resources *r = res();

    // I guess we will just have to locate the shared pointer ourselves
    auto pos = std::find_if(r->models.begin(), r->models.end(), [&](std::shared_ptr<Model> m)
    {
        return m.get() == model;
    });
    assert(pos != r->models.end() && "The model pointer is not present in resources");

    std::shared_ptr<BVH> bvh = std::make_shared<BVH>(*pos);
    r->bvhs.push_back(bvh);
    return bvh.get();
}

TriC *bvh_get_tri(const BVH *bvh, int index)
{
    assert(index >= 0 && index < bvh->get_num_triangles() && "BVH triangle index out of bounds");
    return (TriC *) &bvh->get_triangle(index);
}

int bvh_tri_count(const BVH *bvh)
{
    return bvh->get_num_triangles();
}

TriC *bvh_get_emitter(const BVH *bvh, int index)
{
    assert(index >= 0 && index < bvh->get_num_emitters() && "BVH emitter index out of bounds");
    return (TriC *) &bvh->get_emitter(index);
}

int bvh_emitter_count(const BVH *bvh)
{
    return bvh->get_num_emitters();
}

int bvh_push_node(BVH *bvh, const BBox &bbox, int start, int size, int l, int r)
{
    return bvh->make_node(bbox, start, size, l, r);
}

int bvh_node_count(const BVH *bvh)
{
    return bvh->get_num_nodes();
}

const Node bvh_get_node(BVH *bvh, int index)
{
    assert(index >= 0 && index < bvh->get_num_nodes() && "BVH node index out of bounds");
    return bvh->get_node(index);
}

void bvh_node_set_children(BVH *bvh, int who, int l, int r)
{
    bvh->set_children(who, l, r);
}

void free_bvh(BVH *bvh)
{
    Resources *r = res();
    auto it = std::find_if(r->bvhs.begin(), r->bvhs.end(), [&](const std::shared_ptr<BVH> b)
    {
        return b.get() == bvh;
    });
    assert(it != r->bvhs.end() && "Non-existent BVH");
    r->bvhs.erase(it, it + 1); // WARNING: bvh now becomes a dangling pointer
}

bool *make_partitioning_table(BVH *bvh)
{
    return new bool[bvh->get_num_triangles()];
}

int partition(BVH *bvh, bool *table, int begin, int end)
{
    int sep = bvh->partition(table, begin, end);
    delete[] table;
    return sep;
}

void shade(int width, int height, const char *path)
{
    res()->parallel_launcher(width, height, path);
}

void inventory_add(const char *k, void *v)
{
    res()->inventory_add(k, v);
}

void *inventory_get(const char *k)
{
    return res()->inventory_get(k);
}

void inventory_clear()
{
    res()->inventory_clear();
}

void shared_add(const char *k, const char *serialized, int size)
{
    char *mem = new char[size];
    std::memcpy(mem, serialized, size);
    res()->shared_add(k, { mem, size });
}

const SharedInfo *shared_get(const char *k)
{
    return res()->shared_get(k);
}

void shared_clear()
{
    res()->shared_clear();
}

void debug()
{
    Resources *r = res();
    std::cout << "Whelp." << std::endl;
}

HitInfo model_hit_info(Model *model, int material_id, Vec2C uv)
{
    return model->get_hit_info(material_id, *((glm::vec2 *) &uv));
}

