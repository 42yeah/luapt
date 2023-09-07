cmake_minimum_required(VERSION 3.0)

project(luapt)

option(USE_LUAJIT "Use LuaJIT" ON)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(luapt main.cpp image.cpp luaenv.cpp tiny_obj_loader.cc model.cpp glad.c app.cpp modelgl.cpp job.cpp imagegl.cpp shadergl.cpp)

if (USE_LUAJIT)
    find_package(PkgConfig REQUIRED)
    set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig")
    pkg_check_modules(LuaJIT REQUIRED luajit)
else()
    add_subdirectory(lua-5.4.6)
endif()

add_subdirectory(imgui)

target_include_directories(luapt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(luapt PUBLIC glfw glm::glm imgui)

if (USE_LUAJIT)
    target_include_directories(luapt PUBLIC ${LuaJIT_INCLUDE_DIRS})
    target_link_libraries(luapt PUBLIC ${LuaJIT_LIBRARIES})
    target_compile_definitions(luapt PUBLIC USE_LUAJIT)
else()
    target_link_libraries(luapt PUBLIC lua)
endif()

install(TARGETS luapt RUNTIME DESTINATION bin)
