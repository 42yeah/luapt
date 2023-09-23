// SPDX-FileCopyrightText: 2023 Hao Joe <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESOURCES_H
#define RESOURCES_H

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include "image.h"
#include "model.h"
#include "bbox.h"
#define MAX_ERR_LOG_SIZE 128


struct SharedInfo
{
    char *serialized;
    int size;
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

    /**
     * Globally shared C/C++ resources. This can either be a texture (Image), a mesh (Model), or a
     * bounding volume hierarchy (BVH). I am not sure if custom binary data fits.
     */
    void inventory_add(const std::string &key, void *what);
    void *inventory_get(const std::string &key);
    void inventory_clear();

    /**
     * Globally shared strings. This can be serialized Lua tables using LuaJIT stringbuffers.
     */
    void shared_add(const std::string &key, const SharedInfo &info);
    const SharedInfo *shared_get(const std::string &key) const;
    void shared_clear();

private:
    std::vector<std::string> err_log;
    std::map<std::string, void *> inventory;

    // Shared, serialized tables (or what not) using LuaJIT stringbuffer library.
    std::map<std::string, SharedInfo> shared;

    std::mutex mu;
};

Resources *res(); // VERY shorthand function to get the Lua instance.


#endif // RESOURCES_H
