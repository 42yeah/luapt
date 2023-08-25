// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LUAENV_H
#define LUAENV_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
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
     * No copy constructor
     */
    Lua(const Lua &another) = delete;

    /**
     * Destructor
     */
    ~Lua();

    int execute(const std::string &buffer);
    int execute_file(const std::string &file);

    void report_error(const std::string &msg);
    void register_funcs();

    const std::vector<std::shared_ptr<Image> > &get_images() const;
    const std::vector<std::shared_ptr<Model> > &get_models() const;

    std::vector<std::string> err_log;

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

    static std::shared_ptr<Lua> inst();

private:
    /**
     * Default constructor
     * Thou shalt not call me directly, for I am a singleton
     */
    Lua();

    bool lua_ready;
    lua_State *l;
    std::vector<std::shared_ptr<Image> > images;
    std::vector<std::shared_ptr<Model> > models;
};

#endif // LUA_H
