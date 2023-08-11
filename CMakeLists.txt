cmake_minimum_required(VERSION 3.0)

project(luapt)

find_package(glfw3 REQUIRED)

add_subdirectory(lua-5.4.6)

add_executable(luapt main.cpp image.cpp luaenv.cpp)

target_include_directories(luapt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(luapt PUBLIC glfw lua)

install(TARGETS luapt RUNTIME DESTINATION bin)
