// Job description of what to execute.
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JOB_H
#define JOB_H

#include <string>
#include <iostream>

enum class JobType
{
    Nothing, Suicide, RunScript, Execute, ExecuteParallel
};

struct ParallelParams
{
    ParallelParams() : u(0.0f), v(0.0f), x(0), y(0), w(0), h(0), bytecode(""), image_handle(-1)
    {

    }

    ParallelParams(float u, float v, int x, int y, int w, int h, const std::string &bytecode, int image_handle) : u(u), v(v), x(x), y(y), w(w), h(h), bytecode(bytecode), image_handle(image_handle)
    {

    }

    float u, v;
    int x, y, w, h;
    std::string bytecode;
    int image_handle;
};

/**
 * Job description of what to execute.
 */
class Job
{
public:
    /**
     * Default constructor
     */
    Job(JobType type, const ParallelParams& pparams, const std::string& script_path = "", const std::string& code_injection = "");

    /**
     * Copy constructor
     *
     * @param other the other job.
     */
    Job(const Job& other) = default;

    /**
     * Destructor
     */
    ~Job() = default;

    JobType get_job_type() const;
    const std::string &get_script_path() const;
    const std::string &get_code_injection() const;
    const ParallelParams &get_parallel_params() const;

private:
    JobType type;
    std::string script_path;
    std::string code_injection;
    ParallelParams pparams;
};

struct WorkerStats
{
    int id;
    bool alive;
    bool idle;
    Job *current_job;
};

#endif // JOB_H
