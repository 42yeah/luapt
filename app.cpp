// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include "luaenv.h"

int thread_id_counter = 0;

App::App(GLFWwindow *window) : window(window), w(0), h(0), initialized(false), alive(true), batch_job_count(0), done_job_count(0), ui_show_resources(false), ui_show_lua(false), current_script_path(""), code_injection(""), viewing_image_idx(-1), viewing_model_idx(-1), viewing_bvh_idx(-1), is_wayland(false), display_rect(nullptr), image_viewing_shader(nullptr), showing_image(nullptr), lua(std::make_shared<Lua>())
{

}

bool App::init()
{
    update_framebuffer_size();

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // No Ini

    ImGui::StyleColorsDark();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        std::cerr << "Cannot initialize ImGui GLFW backend." << std::endl;
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330 core"))
    {
        std::cerr << "Cannot initialize ImGui OpenGL3 backend." << std::endl;
        return false;
    }

    // Determine if we are using Wayland + GPU. This will have the mysterious effect of doubling the size of the framebuffer.
    const char *disp = std::getenv("WAYLAND_DISPLAY");
    if (disp)
    {
        is_wayland = true;
    }

    // Initialize the display rectangle.
    Vertex corners[4] = {
        Vertex{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f), glm::vec2(0.0f) },
        Vertex{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f), glm::vec2(0.0f) },
        Vertex{ glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f), glm::vec2(0.0f) },
        Vertex{ glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f), glm::vec2(0.0f) }
    };
    std::vector<Triangle> rect = {{
        Triangle{ corners[0], corners[1], corners[2] },
        Triangle{ corners[2], corners[3], corners[0] }
    }};
    std::shared_ptr<Model> model = std::make_shared<Model>(rect);
    display_rect = std::make_shared<ModelGL>(model);

    image_viewing_shader = std::make_shared<ShaderGL>();
    if (!image_viewing_shader->link_from_files("shaders/rect.vs", "shaders/rect.fs"))
    {
        return false;
    }
    showing_image = std::make_shared<ImageGL>();

    // Launch threads.
    int con = std::thread::hardware_concurrency();
    if (con == 0)
    {
        std::cerr << "WARNING: No hardware hardware concurrency? One thread will be launched regardless to separate Lua from UI." << std::endl;
        con = 1;
    }
    for (int i = 0; i < con; i++)
    {
        launch_new_thread();
    }

    initialized = true;

    // HACK: this implicitly makes App a singleton
    res()->parallel_launcher = [&](int w, int h, const std::string &path)
    {
        queue_batch_job(w, h, path, true);
    };

    return true;
}


App::~App()
{
    {
        std::lock_guard<std::mutex> lk(mu);
        alive = false;
    }
    cv.notify_all();

    if (!initialized)
    {
        return;
    }

    for (int i = 0; i < threads.size(); i++)
    {
        threads[i].join();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void App::run()
{
    assert(initialized && "App is not initialized");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        render_frame();
        glfwSwapBuffers(window);
    }
}

void App::render_frame()
{
    assert(initialized && "App is not initialized");

    glClearColor(1.0f, 0.5f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the rectangle.
    image_viewing_shader->use();
    glUniform2f((GLuint) image_viewing_shader->get_location("resolution"), (float) w, (float) h);
    if (viewing_image_idx != -1)
    {
        // We are viewing an image. Sync that into our ImageGL, and pass that as uniform.

        showing_image->import_from_image(res()->images[viewing_image_idx]);
        showing_image->bind();
        glActiveTexture(GL_TEXTURE0);
        glUniform1i((GLuint) image_viewing_shader->get_location("image"), 0);
        glUniform1i((GLuint) image_viewing_shader->get_location("hasImage"), 1);
    }
    else
    {
        glUniform1i((GLuint) image_viewing_shader->get_location("hasImage"), 0);
    }
    display_rect->draw();

    render_ui();
}

void App::render_ui()
{
    update_framebuffer_size();
    if (is_wayland)
    {
        glViewport(0, 0, 2 * w, 2 * h);
    }
    else
    {
        glViewport(0, 0, w, h);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    // Left side: resource viewer
    constexpr int init_resviewer_w = 200;
    constexpr int bot_bar_h = 30;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(init_resviewer_w, h - bot_bar_h), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Resource viewer", &ui_show_resources))
    {
        if (ImGui::BeginListBox("Images"))
        {
            char name[MAX_INPUT_CHAR_LENGTH] = { 0 };
            for (int i = 0; i < res()->images.size(); i++)
            {
                std::sprintf(name, "Image %d", res()->images[i]->id());
                if (ImGui::Selectable(name, viewing_image_idx == i))
                {
                    viewing_image_idx = i;
                    viewing_model_idx = -1;
                    viewing_bvh_idx = -1;
                }
            }
            ImGui::EndListBox();
        }
        if (ImGui::BeginListBox("Scenes"))
        {
            char name[MAX_INPUT_CHAR_LENGTH] = { 0 };
            for (int i = 0; i < res()->models.size(); i++)
            {
                std::sprintf(name, "Scene %d", res()->models[i]->id());
                if (ImGui::Selectable(name, viewing_model_idx == i))
                {
                    // Show the scene
                    viewing_model_idx = i;
                    viewing_image_idx = -1;
                    viewing_bvh_idx = -1;
                }
            }
            ImGui::EndListBox();
        }
        if (ImGui::BeginListBox("BVHs"))
        {
            char name[MAX_INPUT_CHAR_LENGTH] = { 0 };
            for (int i = 0; i < res()->bvhs.size(); i++)
            {
                std::sprintf(name, "BVH %d", res()->bvhs[i]->id());
                if (ImGui::Selectable(name, viewing_bvh_idx == i))
                {
                    // Show the scene
                    viewing_model_idx = -1;
                    viewing_image_idx = -1;
                    viewing_bvh_idx = -i;
                }
            }
            ImGui::EndListBox();
        }

        double xpos = 0.0, ypos = 0.0;
        glfwGetCursorPos(window, &xpos, &ypos);
        ypos = h - ypos;
        // Evaluate the transformed & aspect corrected UVs
        float upos = (xpos / w) * 2.0f - 1.0f, vpos = (ypos / h) * 2.0f - 1.0f;
        upos *= ((float) w / h);
        upos = upos * 0.5f + 0.5f;
        vpos = vpos * 0.5f + 0.5f;
        if (showing_image->get_base_image())
        {
            RGB<float> rgb = showing_image->get_base_image()->sample_rgb(upos, vpos, SampleMethod::Repeat);
            ImGui::Text("Current sample");
            ImGui::Text("u %f, v %f", upos, vpos);
            ImGui::Text("r %f, g %f, b %f", rgb.r, rgb.g, rgb.b);
            ImGui::Text("r %d, g %d, b %d", (int) (rgb.r * 255.0f), (int) (rgb.g * 255.0f), (int) (rgb.b * 255.0f));
        }
    }
    ImGui::End();

    // Right side: Lua executor
    const bool busy = is_busy();
    constexpr int init_luaviewer_w = 400;
    ImGui::SetNextWindowPos(ImVec2(w - init_luaviewer_w, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(init_luaviewer_w, h - bot_bar_h), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Lua", &ui_show_lua))
    {
        if (busy)
        {
            ImGui::BeginDisabled();
        }

        ImGui::InputText("Script", current_script_path, sizeof(current_script_path)); ImGui::SameLine();
        if (ImGui::Button("Execute##1") && strnlen(current_script_path, MAX_INPUT_CHAR_LENGTH) != 0)
        {
            std::string path_str(current_script_path);
            queue_single_job(Job(JobType::RunScript, {}, path_str));
            if (std::find(previous_scripts.begin(), previous_scripts.end(), path_str) == previous_scripts.end())
            {
                previous_scripts.push_back(path_str);
            }
        }
        ImGui::InputText("Inject", code_injection, sizeof(code_injection)); ImGui::SameLine();
        if (ImGui::Button("Execute##2") && strnlen(code_injection, MAX_INPUT_CHAR_LENGTH) != 0)
        {
            queue_single_job(Job(JobType::Execute, {}, "", code_injection));
        }

        if (busy)
        {
            ImGui::EndDisabled();
        }

        if (ImGui::BeginListBox("Errors"))
        {
            for (auto err = res()->err_begin(); err != res()->err_end(); err++)
            {
                ImGui::Selectable(err->c_str());
            }
            ImGui::EndListBox();
        }
        if (ImGui::Button("Clear Errors"))
        {
            res()->clear_error();
        }
        if (ImGui::BeginListBox("Script History"))
        {
            for (int i = previous_scripts.size() - 1; i >= 0; i--)
            {
                if (ImGui::Selectable(previous_scripts[i].c_str()) && !busy)
                {
                    queue_single_job(Job(JobType::RunScript, {}, previous_scripts[i]));
                }
            }
            ImGui::EndListBox();
        }
        if (ImGui::Button("Clear Script History"))
        {
            previous_scripts.clear();
        }

        ImGui::Text("Worker stats");
        if (ImGui::BeginTable("stats", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Worker ID");
            ImGui::TableSetupColumn("Idleness");
            ImGui::TableHeadersRow();

            for (const auto &stat : worker_stats)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::Text("Thread %d", stat.second.id);
                ImGui::TableSetColumnIndex(1);
                if (!stat.second.idle)
                {
                    ImGui::Text("Busy");
                }
                else
                {
                    ImGui::Text("Idle");
                }
            }
            ImGui::EndTable();
        }
        if (ImGui::Button("Wipe states") && !busy)
        {
            lua = std::make_shared<Lua>();
            for (int i = 0; i < worker_stats.size(); i++)
            {
                Job j(JobType::Reset, ParallelParams(), "", "", worker_stats[i].id);
                queue_single_job(j);
            }

            std::lock_guard<std::mutex> lk(mu);
            batch_job_count = worker_stats.size();
            done_job_count = 0;

            res()->inventory_clear();
            res()->shared_clear();
        }
        if (ImGui::Button("Abort"))
        {
            std::lock_guard<std::mutex> lk(mu);
            jobs = std::queue<Job>(); // Emergency stop
            batch_job_count = 0;
            done_job_count = 0;
        }
    }
    ImGui::End();

    // Render the bottom bar.
    ImGui::SetNextWindowPos(ImVec2(0, h - bot_bar_h), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(w, bot_bar_h), ImGuiCond_Always);
    ImGui::Begin("Bottom bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    if (batch_job_count == done_job_count)
    {
        batch_job_count = 0;
        done_job_count = 0;
        ImGui::Text("Idle");
    }
    else if (batch_job_count == 1 && done_job_count == 0)
    {
        ImGui::Text("Running...");
    }
    else if (done_job_count > batch_job_count)
    {
        ImGui::Text("Unknown");
    }
    else
    {
        float done_percent = (float) done_job_count / batch_job_count;
        ImGui::Text("Running: %f%%...", done_percent * 100.0f);
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(done_percent * w, h), IM_COL32(12, 235, 31, 35));
    }
    ImGui::End();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::launch_new_thread()
{
    int id = thread_id_counter++;
    threads.push_back(std::thread(worker_thread, std::ref(*this), id));
}

void App::worker_thread(App &app, int thread_id)
{
    std::shared_ptr<Lua> lua_clone = std::make_shared<Lua>();
    app.worker_stats[thread_id] = WorkerStats{};
    WorkerStats &me = app.worker_stats[thread_id];

    {
        std::lock_guard<std::mutex> lk(app.mu);
        me.id = thread_id;
        me.alive = true;
        me.current_job = nullptr;
        me.idle = true;
    }

    while (app.alive)
    {
        std::unique_lock<std::mutex> lk(app.mu);
        app.cv.wait(lk, [&]()
        {
            if (!app.alive)
            {
                return true;
            }
            bool empty = app.jobs.empty();
            if (!empty)
            {
                const Job &f = app.jobs.front();
                int tar = f.get_target_worker();
                return tar == -1 || tar == thread_id;
            }
            return false;
        });
        if (!app.alive)
        {
            std::cout << "Worker #" << thread_id << " is bidding farewell." << std::endl;
            me.alive = false;
            return;
        }

        // Take a job. Any job.
        Job job = app.jobs.front();
        app.jobs.pop();

        // std::cout << "Worker #" << thread_id << ": job popped. Remaining jobs: " << app.jobs.size() << std::endl;
        me.current_job = &job;
        me.idle = false;
        lk.unlock();

        switch (job.get_job_type())
        {
            case JobType::Nothing:
                break;

            case JobType::Suicide:
                return;

            case JobType::RunScript:
                app.lua->execute_file(job.get_script_path());
                break;

            case JobType::Execute:
                app.lua->execute(job.get_code_injection());
                break;

            case JobType::ExecuteParallel:
            {
                /**
                 * Execute scripts in parallel.
                 * The source code is available in ParallelParams.
                 */
                const ParallelParams &pparams = job.get_parallel_params();
                lua_clone->call_shade(pparams.src, pparams.u, pparams.v, pparams.x, pparams.y, pparams.w, pparams.h);

                break;
            }

            case JobType::Reset:
                lua_clone = std::make_shared<Lua>();
                break;
        }

        {
            std::lock_guard<std::mutex> lk(app.mu);
            me.current_job = nullptr;
            me.idle = true;
            app.done_job_count++;
            // Clamp the done_job_count to avoid unknown state.
            if (app.done_job_count > app.batch_job_count)
            {
                app.done_job_count = app.batch_job_count;
            }

            // If job is done, or this is a ID-specific job, we will have to notify everyone so that they know the next task is coming
            if (app.done_job_count + 1 >= app.batch_job_count || job.get_target_worker() != -1)
            {
                app.cv.notify_all();
            }
            // std::cout << thread_id << ": Done!" << std::endl;
        }
    }
}

void App::queue_single_job(const Job &job)
{
    {
        std::lock_guard<std::mutex> lk(mu);
        batch_job_count = 1;
        done_job_count = 0;
        jobs.push(job);
    }
    cv.notify_one();
}

void App::queue_batch_job(int w, int h, const std::string &path, bool wait_until_finish)
{
    {
        std::lock_guard<std::mutex> lk(mu);
        batch_job_count = w * h + 1;
        done_job_count = 0;

        // Read the source code.
        std::ifstream reader(path);
        if (!reader.good())
        {
            std::stringstream ss;
            ss << "Error: cannot load file: " << path;
            res()->report_error(ss.str());
            batch_job_count = 0;
            done_job_count = 0;
            return;
        }

        std::stringstream ss;
        ss << reader.rdbuf();

        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float u = ((float) x + 0.5f) / w, v = ((float) y + 0.5f) / h;
                Job job(JobType::ExecuteParallel, ParallelParams(u, v, x, y, w, h, ss.str()));
                jobs.push(job);
            }
        }
    }
    cv.notify_all();

    if (wait_until_finish)
    {
        std::unique_lock<std::mutex> lk(mu);
        cv.wait(lk, [&]()
        {
            return !alive || (batch_job_count <= done_job_count + 1);
        });
    }
}

void App::update_framebuffer_size()
{
    if (is_wayland)
    {
        glfwGetWindowSize(window, &w, &h);
    }
    else
    {
        glfwGetFramebufferSize(window, &w, &h);
    }
}

void App::set_script_path(const std::string &path)
{
    std::strncpy(this->current_script_path, path.c_str(), std::min(MAX_INPUT_CHAR_LENGTH, (int) path.size()));
}

bool App::is_busy()
{
    std::lock_guard<std::mutex> lk(mu);
    return batch_job_count != done_job_count;
}
