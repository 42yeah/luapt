// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "job.h"

Job::Job(JobType type, const ParallelParams &pparams, const std::string &script_path, const std::string &code_injection, int target_worker) : type(type), script_path(script_path), code_injection(code_injection), pparams(pparams), target_worker(target_worker)
{

}

JobType Job::get_job_type() const
{
    return type;
}

const std::string &Job::get_script_path() const
{
    return script_path;
}


const std::string &Job::get_code_injection() const
{
    return code_injection;
}

const ParallelParams &Job::get_parallel_params() const
{
    return pparams;
}

int Job::get_target_worker() const
{
    return target_worker;
}
