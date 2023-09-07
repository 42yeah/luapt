// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LUAENV_H
#define LUAENV_H

extern "C"
{
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <functional>
#include <mutex>
#include "image.h"
#include "model.h"
#define MAX_ERR_LOG_SIZE 128

/**
 * The Lua interface. Responsible for loading in scripts and executing them.
 * The whole thing is designed to be a giant state machine, and therefore non-copyable.
 */
class Lua
{
public:
    /**
     * Default constructor
     */
    Lua();

    /**
     * As resources are now an instance of their own, we can officially no longer
     * copy Lua instances.
     */
    Lua(const Lua &another) = delete;

    /**
     * Destructor
     */
    ~Lua();

    int execute(const std::string &buffer);
    int execute_file(const std::string &file);
    void call_shade(const std::string &src, float u, float v, int x, int y, int w, int h);

private:
    bool lua_ready;
    lua_State *l;
};


/**
 * Globally loaded resources. This shall be indexable from anywhere.
 */
class Resources
{
public:
    Resources() = default;
    ~Resources() = default;

    std::vector<std::shared_ptr<Image> > images;
    std::vector<std::shared_ptr<Model> > models;

    // We will try to call this when we need to aunch w*h number of threads.
    // Parameters: w, h, and script path
    std::function<void(int, int, std::string)> parallel_launcher;
    void report_error(const std::string &msg);
    void clear_error();

    /**
     * Obtain the const iterator.
     * This won't lock, because the vector size only goes up when Lua produces errors,
     * and only goes down when we clear it. Which is in the UI thread, so nothing will go wrong
     */
    std::vector<std::string>::const_iterator err_begin() const;
    std::vector<std::string>::const_iterator err_end() const;

    void inventory_add(const std::string &key, void *what);
    void *inventory_get(const std::string &key);
    void inventory_clear();

private:
    std::vector<std::string> err_log;
    std::map<std::string, void *> inventory;
    std::mutex mu;
};


extern "C"
{
    Resources *res(); // VERY shorthand function to get the Lua instance.

    /**
     * BEGIN
     * A more boring and vanilla triangle definition.
     */
    typedef struct
    {
        float x;
        float y;
        float z;
    } Vec3C;

    typedef struct
    {
        float u, v;
    } Vec2C;

    typedef struct
    {
        Vec3C position;
        Vec3C normal;
        Vec2C tex_coord;
    } VertexC;

    typedef struct
    {
        VertexC a, b, c;
    } TriC;
    // END boring triangle definition //

    // Images
    Image *make_image(int width, int height);
    void set_pixel(Image *img, int x, int y, float r, float g, float b);
    bool save_image(Image *img, const char *path);
    void free_image(Image *img);

    // Models
    Model *make_model(const char *path);
    int model_tri_count(const Model *model);
    TriC *model_get_tri(const Model *model, int index);
    void free_model(Model *model);

    // Inventory
    void inventory_add(const char *k, void *v);
    void *inventory_get(const char *k);
    void inventory_clear();

    /**
     * Shade function launches a bunch of threads (width*height).
     * Sure we can dump functions and whatnot like before but that's not really necessary.
     * Since resource is shared in this brand new world, simply passing in a Lua script path
     * is easier, offers finer-grained control, and much more safer.
     */
    void shade(int width, int height, const char *path);
}

#endif // LUA_H
