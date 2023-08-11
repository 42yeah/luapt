cmake_minimum_required(VERSION 3.0)

project(luapt)

add_executable(luapt main.cpp)

install(TARGETS luapt RUNTIME DESTINATION bin)
