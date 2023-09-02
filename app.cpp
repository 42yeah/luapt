// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app.h"
#include <iostream>
#include <cassert>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include "luaenv.h"

int thread_id_counter = 0;

App::App(GLFWwindow *window) : window(window), w(0), h(0), initialized(false), alive(true), batch_job_count(0), done_job_count(0), ui_show_resources(false), ui_show_lua(false), current_script_path(""), code_injection(""), viewing_image_idx(-1), viewing_model_idx(-1), display_rect(nullptr), image_viewing_shader(nullptr), showing_image(nullptr)
{

}

bool App::init()
{
    glfwGetFramebufferSize(window, &w, &h);

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
    lua.parallel_launcher = [&](int w, int h, const std::string &bytecode, int image_handle)
    {
        queue_batch_job(w, h, bytecode, image_handle);
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
        showing_image->import_from_image(*lua.get_images()[viewing_image_idx]);
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
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);

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
            for (int i = 0; i < lua.get_images().size(); i++)
            {
                std::sprintf(name, "Image %d", i);
                if (ImGui::Selectable(name, viewing_image_idx == i))
                {
                    viewing_image_idx = i;
                    viewing_model_idx = -1;
                }
            }
            ImGui::EndListBox();
        }
        if (ImGui::BeginListBox("Scenes"))
        {
            char name[MAX_INPUT_CHAR_LENGTH] = { 0 };
            for (int i = 0; i < lua.get_models().size(); i++)
            {
                std::sprintf(name, "Scene %d", i);
                if (ImGui::Selectable(name, viewing_model_idx == i))
                {
                    // Show the scene
                    viewing_model_idx = i;
                    viewing_image_idx = -1;
                }
            }
            ImGui::EndListBox();
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
            for (int i = 0; i < lua.err_log.size(); i++)
            {
                ImGui::Selectable(lua.err_log.at(i).c_str());
            }
            ImGui::EndListBox();
        }
        if (ImGui::Button("Clear Errors"))
        {
            lua.err_log.clear();
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
            lua.err_log.clear();
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
        ImGui::Text("Running: %f...", done_percent);
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
    std::shared_ptr<Lua> lua_clone = nullptr;
    app.worker_stats[thread_id] = WorkerStats{};
    WorkerStats &me = app.worker_stats[thread_id];

    // Acquire a lua clone (for multithreading)
    // Because of tens of thousands of weird data racing stuffs, we will be creating brand new copies here
    {
        std::lock_guard<std::mutex> lk(app.mu);
        lua_clone = std::make_shared<Lua>(app.lua);

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
            return !app.jobs.empty() || !app.alive;
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
                app.lua.execute_file(job.get_script_path());
                break;

            case JobType::Execute:
                app.lua.execute(job.get_code_injection());
                break;

            case JobType::ExecuteParallel:
            {
                const ParallelParams &pparams = job.get_parallel_params();
                lua_clone->call_shade(pparams.bytecode, pparams.u, pparams.v, pparams.x, pparams.y, pparams.w, pparams.h, pparams.image_handle);

                if (lua_clone->err_log.size() > 0 && app.lua.err_log.size() < MAX_ERR_LOG_SIZE)
                {
                    std::lock_guard<std::mutex> lk(app.mu);
                    for (int i = 0; i < lua_clone->err_log.size(); i++)
                    {
                        app.lua.err_log.push_back(lua_clone->err_log[i]);
                    }
                    lua_clone->err_log.clear();
                }

                break;
            }
        }

        {
            std::lock_guard<std::mutex> lk(app.mu);
            me.current_job = nullptr;
            me.idle = true;
            app.done_job_count++;
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

void App::queue_batch_job(int w, int h, const std::string &bytecode, int image_handle)
{
    {
        std::lock_guard<std::mutex> lk(mu);
        batch_job_count += w * h;
        done_job_count = 0;
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                float u = ((float) x + 0.5f) / w, v = ((float) y + 0.5f) / h;
                Job job(JobType::ExecuteParallel, ParallelParams(u, v, x, y, w, h, bytecode, image_handle));
                jobs.push(job);
            }
        }
    }
    cv.notify_all();
}

bool App::is_busy()
{
    std::lock_guard<std::mutex> lk(mu);
    return batch_job_count != done_job_count;
}
