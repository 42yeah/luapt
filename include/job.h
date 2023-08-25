// Job description of what to execute.
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JOB_H
#define JOB_H

#include <string>
#include <iostream>

enum class JobType
{
    Nothing, Suicide, RunScript, Execute
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
    Job(JobType type, const std::string &script_path = "", const std::string &snippet = "");

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

private:
    JobType type;
    std::string script_path;
    std::string code_injection;
};

#endif // JOB_H
