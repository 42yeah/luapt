// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APP_H
#define APP_H

#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "job.h"
#define MAX_INPUT_CHAR_LENGTH 256

/**
 * App manages both the OpenGL context and the Lua context. Basically the user interface.
 */
class App
{
public:
    /**
     * Default constructor
     */
    App(GLFWwindow *window);

    App(const App& other) = delete;

    /**
     * Destructor
     */
    ~App();

    /**
     * Initializes the App environment.
     */
    bool init();

    void run();

private:
    void render_frame();
    void render_ui();
    void launch_new_thread();
    void queue_single_job(const Job &job);

    /**
     * @returns if a batch job is launched at the moment.
     */
    bool is_busy();

    static void worker_thread(App &app, int thread_id);

    GLFWwindow *window;
    int w, h;
    bool initialized;
    bool alive;

    // Threads
    std::vector<std::thread> threads;
    std::queue<Job> jobs;
    int batch_job_count;
    int done_job_count;
    std::condition_variable cv;
    std::mutex mu;

    // UI flags
    bool ui_show_resources, ui_show_lua;
    char current_script_path[MAX_INPUT_CHAR_LENGTH];
    char code_injection[MAX_INPUT_CHAR_LENGTH];
    std::vector<std::string> previous_scripts;
};

#endif // APP_H
