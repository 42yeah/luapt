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
#include <functional>
#include "image.h"
#include "model.h"

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
     * The copy constructor is used for multithreading.
     * Other instances of Lua will simply inherit the images and models between them, making them all shared.
     * However, each lua_State * is brand new.
     */
    Lua(const Lua &another);

    /**
     * Destructor
     */
    ~Lua();

    int execute(const std::string &buffer);
    int execute_file(const std::string &file);
    void call_shade(const std::string &bytecode, float u, float v, int x, int y, int w, int h, int image_handle);

    void report_error(const std::string &msg);
    void register_funcs();

    const std::vector<std::shared_ptr<Image> > &get_images() const;
    const std::vector<std::shared_ptr<Model> > &get_models() const;

    std::vector<std::string> err_log;

    static Lua *get_self(lua_State *l);

    // Images
    static int make_image(lua_State *l);
    static int set_pixel(lua_State *l);
    static int save_image(lua_State *l);
    static int free_image(lua_State *l);

    // Models
    static int make_model(lua_State *l);
    static int model_tri_count(lua_State *l);
    static int model_get_tri(lua_State *l);
    static int free_model(lua_State *l);
    static int shade(lua_State *l);

    // We will try to call this when we need to aunch w*h number of threads.
    // Parameters: w, h, bytecode, and the image handle
    std::function<void(int, int, std::string, int)> parallel_launcher;

private:
    bool lua_ready;
    lua_State *l;
    bool cloned;

    std::shared_ptr<std::vector<std::shared_ptr<Image> > > images;
    std::shared_ptr<std::vector<std::shared_ptr<Model> > > models;
};

// Some helper functions

/**
 * Copy the current table referenced in the `from` stack to the `to` stack.
 * @param to the destination.
 * @param from the source.
 */
void lua_copy_table(lua_State *to, lua_State *from, int table_index);

/**
 * Copy the current value referenced in the `from` stack to the `to` stack.
 * @param to the destination.
 * @param from the source.
 */
void lua_copy_value(lua_State *to, lua_State *from);

// Switching to FFI??!
extern "C"
{
    void print_hello(int a);
}

#endif // LUA_H
