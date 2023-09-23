// SPDX-FileCopyrightText: 2023 Hao Joe <email>
// SPDX-License-Identifier: Apache-2.0

#include "resources.h"
#include <sstream>


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

void Resources::shared_add(const std::string &key, const SharedInfo &what)
{
    std::lock_guard<std::mutex> lk(mu);
    shared[key] = what;
}

const SharedInfo *Resources::shared_get(const std::string &key) const
{
    auto pos = shared.find(key);
    if (pos == shared.end())
    {
        return nullptr;
    }
    return &pos->second;
}

void Resources::shared_clear()
{
    std::lock_guard<std::mutex> lk(mu);

    for (const auto &k : shared)
    {
        delete[] k.second.serialized;
    }
    shared.clear();
}
