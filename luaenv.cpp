// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "luaenv.h"
#include <cassert>
#include <sstream>
#include <algorithm>
#include <image.h>


Lua::Lua() : lua_ready(false), l(nullptr)
{
    l = luaL_newstate();
    luaL_openlibs(l);

    lua_pushlightuserdata(l, this);
    lua_setglobal(l, "luaenv");

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

int generate_test_gradient_image(lua_State *l)
{
    int w = lua_tointeger(l, 1);
    int h = lua_tointeger(l, 2);
    const char *filename = lua_tostring(l, 3);
    std::shared_ptr<Image> img = generate_gradient_image(w, h);
    img->save(filename);
    return 0;
}

void Resources::report_error(const std::string &msg)
{
    if (msg.empty())
    {
        return;
    }

    std::lock_guard<std::mutex> lk(mu);
    if (err_log.size() > MAX_ERR_LOG_SIZE)
    {
        // Ditch the NEW error log
        return;
    }
    err_log.push_back(msg + "##err" + std::to_string(err_log.size()));
    std::cout << "ERROR: " << msg << std::endl;
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


// FFI Implementations
std::shared_ptr<Resources> res_instance = nullptr;

Resources *res()
{
    if (!res_instance)
    {
        res_instance = std::make_shared<Resources>();
    }
    return res_instance.get();
}


std::vector<std::string>::const_iterator Resources::err_begin() const
{
    return err_log.begin();
}

std::vector<std::string>::const_iterator Resources::err_end() const
{
    return err_log.end();
}

void Resources::clear_error()
{
    std::lock_guard<std::mutex> lk(mu);
    err_log.clear();
}

void Resources::inventory_add(const std::string &key, void *what)
{
    std::lock_guard<std::mutex> lk(mu);
    inventory[key] = what;
}

void *Resources::inventory_get(const std::string &key)
{
    std::lock_guard<std::mutex> lk(mu);
    auto pos = inventory.find(key);
    if (pos == inventory.end())
    {
        std::stringstream ss;
        ss << "Nonexistent inventory key: " << key;
        res()->err_log.push_back(ss.str());
        return nullptr;
    }
    return pos->second;
}

void Resources::inventory_clear()
{
    std::lock_guard<std::mutex> lk(mu);
    inventory.clear();
}


// Images
Image *make_image(int width, int height)
{
    Resources *r = res();
    std::shared_ptr<Image> img = std::make_shared<Image>(width, height, 4);
    r->images.push_back(img);
    return img.get();
}

void set_pixel(Image *img, int x, int y, float r, float g, float b)
{
    img->set_rgb(x, y, ccomp(r), ccomp(g), ccomp(b));
}

bool save_image(Image *img, const char *path)
{
    return img->save(path);
}

void free_image(Image *img)
{
    Resources *r = res();
    auto it = std::find_if(r->images.begin(), r->images.end(), [&](const std::shared_ptr<Image> im)
    {
        return im.get() == img;
    });
    assert(it != r->images.end() && "Non-existent image");
    r->images.erase(it, it + 1); // WARNING: img now becomes a dangling pointer
}


// Models
Model *make_model(const char *path)
{
    std::shared_ptr<Model> model = std::make_shared<Model>();
    if (!model->load(path))
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

void debug()
{
    Resources *r = res();
    std::cout << "Whelp." << std::endl;
}
