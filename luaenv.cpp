// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "luaenv.h"
#include <cassert>
#include <sstream>
#include <algorithm>
#include <image.h>


Lua::Lua() : parallel_launcher(nullptr), lua_ready(false), l(nullptr), cloned(false)
{
    images = std::make_shared<std::vector<std::shared_ptr<Image> > >();
    models = std::make_shared<std::vector<std::shared_ptr<Model> > >();

    l = luaL_newstate();
    luaL_openlibs(l);
    register_funcs();

    lua_pushlightuserdata(l, this);
    lua_setglobal(l, "luaenv");

    if (l)
    {
        lua_ready = true;
    }
}

Lua::Lua(const Lua &another) : parallel_launcher(nullptr), lua_ready(false), l(nullptr), cloned(true)
{
    images = another.images;
    models = another.models;

    l = luaL_newstate();
    luaL_openlibs(l);
    register_funcs();

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
        report_error(ss.str());
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
        report_error(ss.str());
        return 1;
    }

    std::stringstream ss;
    ss << reader.rdbuf();
    return execute(ss.str());
}

void Lua::report_error(const std::string &msg)
{
    assert(lua_ready && l && "Lua is not ready");

    err_log.push_back(msg);
    std::cout << "ERROR: " << msg << std::endl;
}

const std::vector<std::shared_ptr<Image> > &Lua::get_images() const
{
    return *images;
}

const std::vector<std::shared_ptr<Model> > &Lua::get_models() const
{
    return *models;
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

Lua *Lua::get_self(lua_State *l)
{
#ifdef USE_LUAJIT
    lua_getglobal(l, "luaenv");
#else
    assert(lua_getglobal(l, "luaenv") != 0 && "luaenv not found");
#endif
    Lua *self = (Lua *) (lua_touserdata(l, -1));
    lua_pop(l, 1);
    return self;
}

int Lua::make_image(lua_State *l)
{
    Lua *self = get_self(l);

    int w = lua_tointeger(l, 1);
    int h = lua_tointeger(l, 2);
    std::shared_ptr<Image> img = std::make_shared<Image>(w, h, 4);
    self->images->push_back(img);
    lua_pushinteger(l, img->id());
    return 1;
}

int Lua::set_pixel(lua_State *l)
{
    Lua *self = get_self(l);

    int id = lua_tointeger(l, 1);
    int x = lua_tointeger(l, 2);
    int y = lua_tointeger(l, 3);
    float r = lua_tonumber(l, 4);
    float g = lua_tonumber(l, 5);
    float b = lua_tonumber(l, 6);

    auto it = std::find_if(self->images->begin(), self->images->end(), [&](const std::shared_ptr<Image> &img)
        {
            return img->id() == id;
        }
    );

    // Check if the image exists.
    if (it == self->images->end())
    {
        luaL_error(l, "Image handle not found: %d.", id);
        return 0;
    }

    // Check if out of bounds.
    if (x < 0 || x >= (*it)->w || y < 0 || y >= (*it)->h)
    {
        luaL_error(l, "Sample out of bounds: (%d, %d). Image size: (%d, %d).", x, y, (*it)->w, (*it)->h);
        return 0;
    }

    (*it)->set_rgb(x, y, ccomp(r), ccomp(g), ccomp(b));

    return 0;
}

int Lua::save_image(lua_State *l)
{
    Lua *self = get_self(l);

    int id = lua_tointeger(l, 1);
    const char *path = lua_tostring(l, 2);
    auto it = std::find_if(self->images->begin(), self->images->end(), [&](const std::shared_ptr<Image> &img)
        {
            return img->id() == id;
        }
    );

    // Check if the image exists.
    if (it == self->images->end())
    {
        luaL_error(l, "Image handle not found: %d.", id);
        return 0;
    }

    bool res = (*it)->save(path);
    if (!res)
    {
        luaL_error(l, "Cannot save image: %s.", path);
    }
    return 0;
}

int Lua::free_image(lua_State *l)
{
    Lua *self = get_self(l);

    int id = lua_tointeger(l, 1);
    auto it = std::find_if(self->images->begin(), self->images->end(), [&](const std::shared_ptr<Image> &img)
        {
            return img->id() == id;
        }
    );

    // Check if the image exists.
    if (it == self->images->end())
    {
        luaL_error(l, "Image handle not found: %d.", id);
        return 0;
    }

    self->images->erase(it);
    return 0;
}

int Lua::make_model(lua_State *l)
{
    Lua *self = get_self(l);

    const char *path = lua_tostring(l, 1);

    std::shared_ptr<Model> model = std::make_shared<Model>();
    if (!model->load(path))
    {
        luaL_error(l, "Cannot load model: %s", model->get_load_errors().c_str());
        return 0;
    }

    std::string warnings = model->get_load_warnings();
    if (!warnings.empty())
    {
        std::cerr << "WARNING! " << warnings << std::endl;
    }

    self->models->push_back(model);
    lua_pushinteger(l, model->id());
    return 1;
}

int Lua::model_tri_count(lua_State *l)
{
    Lua *self = get_self(l);

    int id = lua_tointeger(l, 1);
    auto it = std::find_if(self->models->begin(), self->models->end(), [&](const std::shared_ptr<Model> &model)
        {
            return model->id() == id;
        }
    );

    // Check if the model exists.
    if (it == self->models->end())
    {
        luaL_error(l, "Model handle not found: %d.", id);
        return 0;
    }

    lua_pushinteger(l, (*it)->get_num_tris());
    return 1;
}

int Lua::model_get_tri(lua_State *l)
{
    Lua *self = get_self(l);

    int id = lua_tointeger(l, 1);
    int index = lua_tointeger(l, 2);

    auto it = std::find_if(self->models->begin(), self->models->end(), [&](const std::shared_ptr<Model> &model)
        {
            return model->id() == id;
        }
    );
    if (it == self->models->end())
    {
        luaL_error(l, "Model handle not found: %d.", id);
        return 0;
    }

    const Triangle &tri = (*it)->get_triangle(index);
    if (index < 0 || index >= (*it)->get_num_tris())
    {
        luaL_error(l, "Triangle index out of bounds: %d.", index);
        return 0;
    }

    lua_pushnumber(l, tri.a.position.x);
    lua_pushnumber(l, tri.a.position.y);
    lua_pushnumber(l, tri.a.position.z);
    lua_pushnumber(l, tri.a.normal.x);
    lua_pushnumber(l, tri.a.normal.y);
    lua_pushnumber(l, tri.a.normal.z);
    lua_pushnumber(l, tri.a.tex_coord.x);
    lua_pushnumber(l, tri.a.tex_coord.y);

    lua_pushnumber(l, tri.b.position.x);
    lua_pushnumber(l, tri.b.position.y);
    lua_pushnumber(l, tri.b.position.z);
    lua_pushnumber(l, tri.b.normal.x);
    lua_pushnumber(l, tri.b.normal.y);
    lua_pushnumber(l, tri.b.normal.z);
    lua_pushnumber(l, tri.b.tex_coord.x);
    lua_pushnumber(l, tri.b.tex_coord.y);

    lua_pushnumber(l, tri.c.position.x);
    lua_pushnumber(l, tri.c.position.y);
    lua_pushnumber(l, tri.c.position.z);
    lua_pushnumber(l, tri.c.normal.x);
    lua_pushnumber(l, tri.c.normal.y);
    lua_pushnumber(l, tri.c.normal.z);
    lua_pushnumber(l, tri.c.tex_coord.x);
    lua_pushnumber(l, tri.c.tex_coord.y);

    return 24;
}

int Lua::free_model(lua_State *l)
{
    Lua *self = get_self(l);

    int id = lua_tointeger(l, 1);

    auto it = std::find_if(self->models->begin(), self->models->end(), [&](const std::shared_ptr<Model> &model)
        {
            return model->id() == id;
        }
    );

    // Check if the model exists.
    if (it == self->models->end())
    {
        luaL_error(l, "Model handle not found: %d.", id);
        return 0;
    }

    self->models->erase(it);
    return 0;
}

int Lua::shade(lua_State *l)
{
    Lua *self = get_self(l);
    assert(self->parallel_launcher && "Parallel computation not supported");

    int w = lua_tonumber(l, 1);
    int h = lua_tonumber(l, 2);
    int img_handle = lua_tointeger(l, 3);
    // lua_pushstring(l, "handle");
    // lua_gettable(l, 3);
    // int img_handle = lua_tointeger(l, -1);
    // lua_pop(l, 1);
    int callback_ref = luaL_ref(l, LUA_REGISTRYINDEX);

    // Dump the function
    lua_rawgeti(l, LUA_REGISTRYINDEX, callback_ref);
    std::string buffer;
    auto writer = [](lua_State *l, const void *p, size_t size, void *ud)
    {
        std::string *buf = (std::string *) ud;
        buf->append((const char *) p, size);
        return 0;
    };

#ifdef USE_LUAJIT
    assert(lua_dump(l, writer, &buffer) == 0 && "Failed to dump Lua source code");
#else
    assert(lua_dump(l, writer, &buffer, true) == 0 && "Failed to dump Lua source code");
#endif

    self->parallel_launcher(w, h, buffer, img_handle);

    return 0;
}

void Lua::call_shade(const std::string &bytecode, float u, float v, int x, int y, int w, int h, int image_handle)
{
    assert(lua_ready && l && "Lua is not ready");

    int error = luaL_loadbuffer(l, bytecode.c_str(), bytecode.size(), "shade");
    if (error)
    {
        std::stringstream err;
        err << "Failed to load bytecode: " << lua_tostring(l, -1);
        err_log.push_back(err.str());
    }

    lua_pushnumber(l, u);
    lua_pushnumber(l, v);
    lua_pushinteger(l, x);
    lua_pushinteger(l, y);
    lua_pushinteger(l, w);
    lua_pushinteger(l, h);
    lua_pushinteger(l, image_handle);

    if (lua_pcall(l, 7, 0, 0))
    {
        std::stringstream err;
        err << "Failed to call function: " << lua_tostring(l, -1);
        err_log.push_back(err.str());
    }
}

void Lua::register_funcs()
{
    lua_register(l, "generate_test_gradient_image", generate_test_gradient_image);
    lua_register(l, "make_image", Lua::make_image);
    lua_register(l, "set_pixel", Lua::set_pixel);
    lua_register(l, "save_image", Lua::save_image);
    lua_register(l, "free_image", Lua::free_image);

    lua_register(l, "make_model", Lua::make_model);
    lua_register(l, "free_model", Lua::free_model);
    lua_register(l, "model_tri_count", Lua::model_tri_count);
    lua_register(l, "model_get_tri", Lua::model_get_tri);
    lua_register(l, "shade", Lua::shade);
}

#ifndef USE_LUAJIT
void lua_copy_table(lua_State *to, lua_State *from, int table_index)
{
    lua_newtable(to);
    lua_pushnil(from);
    while (lua_next(from, table_index) != 0)
    {
        lua_copy_value(to, from); // Copy value
        lua_copy_value(to, from); // Copy key
        lua_settable(to, -3);
        lua_pop(from, 1);
    }
}

void lua_copy_value(lua_State *to, lua_State *from)
{
    int type = lua_type(from, -1);
    switch (type)
    {
        case LUA_TNIL:
            lua_pushnil(to);
            break;

        case LUA_TBOOLEAN:
            lua_pushboolean(to, lua_toboolean(from, -1));
            break;

        case LUA_TNUMBER:
            lua_pushnumber(to, lua_tonumber(from, -1));
            break;

        case LUA_TSTRING:
            lua_pushstring(to, lua_tostring(from, -1));
            break;

        case LUA_TTABLE:
            lua_copy_table(to, from, -1);
            break;

        default:
            assert(false && "Unknown type");
            break;
    }
}
#endif

void print_hello(int a)
{
    std::cout << "Hello fucking world! " << a << std::endl;
}
