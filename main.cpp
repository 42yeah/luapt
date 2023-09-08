#include <iostream>
#include <string>
#include <cassert>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "app.h"

#define WINDOW_TITLE "LuaPT User Interface"

constexpr int width = 1280, height = 720;

/**
 * Purpose of main:
 * The purpose of the main() function is to start up a window, then move it to App(), which manages everything, including
 * the UI, and the Lua environment.
 */
int main(int argc, char *argv[])
{
    assert(glfwInit() == GLFW_TRUE && "Cannot initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(width, height, WINDOW_TITLE, nullptr, nullptr);
    assert(window && "Cannot create window");

    glfwMakeContextCurrent(window);
    assert(gladLoadGL() && "Cannot load OpenGL functions.");

    {
        App app(window);

        assert(app.init() && "App initialization failed.");
        if (argc != 1)
        {
            std::stringstream joined;
            for (int i = 1; i < argc; i++)
            {
                if (i != 1)
                {
                    joined << " ";
                }
                joined << argv[i];
            }
            app.set_script_path(joined.str());
        }

        app.run();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
