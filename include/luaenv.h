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
#include "luamath.h"
#include "bbox.h"
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

    std::vector<std::shared_ptr<BaseImage> > images;
    std::vector<std::shared_ptr<Model> > models;
    std::vector<std::shared_ptr<BVH> > bvhs;

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

Resources *res(); // VERY shorthand function to get the Lua instance.

extern "C"
{
    // Images
    U8Image *make_image(int width, int height);
    void set_pixel(U8Image *img, int x, int y, float r, float g, float b);
    bool save_image(U8Image *img, const char *path);
    void free_image(U8Image *img);
    void generate_demo_image(int w, int h, const char *path);

    // Models
    Model *make_model(const char *path, const char *mtl_base_path);
    int model_tri_count(const Model *model);
    TriC *model_get_tri(const Model *model, int index);
    void free_model(Model *model);

    // BVHs
    BVH *make_bvh(Model *model);
    TriC *bvh_get_tri(const BVH *bvh, int index);
    int bvh_tri_count(const BVH *bvh);
    int bvh_push_node(BVH *bvh, const BBox &bbox, int start, int size, int l, int r);
    const Node bvh_get_node(BVH *bvh, int index);
    void bvh_node_set_children(BVH *bvh, int who, int l, int r);
    int bvh_node_count(const BVH *bvh);
    void free_bvh(BVH *bvh);

    /**
     * Returns an array of booleans. This will be used to partition the array.
     * The partitioning array __will be freed__ upon calling `partition`.
     */
    bool *make_partitioning_table(BVH *bvh);
    int partition(BVH *bvh, bool *table, int begin, int end);

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

    /**
     * Trigger the debugger.
     * Insert your own breakpoint here!
     */
    void debug();
}

#endif // LUA_H
