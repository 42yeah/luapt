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
#include <random>
#include "image.h"
#include "model.h"
#include "luamath.h"
#include "bbox.h"
#include "resources.h"

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

    static Lua *get_self(lua_State *l);
    static int whatever(lua_State *l);

private:
    bool lua_ready;
    lua_State *l;
    std::random_device dev;
    std::uniform_real_distribution<float> distrib;
};


extern "C"
{
    // Images
    FloatImage *make_image(int width, int height);
    FloatImage *load_image(const char *path);
    void set_pixel(FloatImage *img, int x, int y, float r, float g, float b);
    bool save_image(FloatImage *img, const char *path);
    void free_image(FloatImage *img);
    Vec3C get_pixel(FloatImage *img, int x, int y);
    Vec3C sample_image(FloatImage *img, float u, float v);
    void generate_demo_image(int w, int h, const char *path);

    // Models
    Model *make_model(const char *path, const char *mtl_base_path);
    int model_tri_count(const Model *model);
    TriC *model_get_tri(const Model *model, int index);
    void free_model(Model *model);
    HitInfo model_hit_info(Model *model, int material_id, Vec2C uv);

    // BVHs
    BVH *make_bvh(Model *model);
    TriC *bvh_get_tri(const BVH *bvh, int index);
    int bvh_tri_count(const BVH *bvh);
    TriC *bvh_get_emitter(const BVH *bvh, int index);
    int bvh_emitter_count(const BVH *bvh);
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

    // Shared strings
    void shared_add(const char *k, const char *serialized, int size);
    const SharedInfo *shared_get(const char *k);
    void shared_clear();

    /**
     * Shade function launches a bunch of threads (width*height).
     * Sure we can dump functions and whatnot like before but that's not really necessary.
     * Since resource is shared in this brand new world, simply passing in a Lua script path
     * is easier, offers finer-grained control, and much more safer.
     */
    void shade(int width, int height, const char *path);

    /**
     * TODO: Add one single extra job (to be traced.)
     * This can be used in, for example, Russian-Roulette situations.
     */
    void add_one_job(float u, float v, int x, int y, int w, int h, const std::string &src);

    /**
     * Trigger the debugger.
     * Insert your own breakpoint here!
     */
    void debug();
}

#endif // LUA_H
