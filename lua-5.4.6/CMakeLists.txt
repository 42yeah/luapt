file(GLOB luafiles src/*.c)
set_source_files_properties(${luafiles} PROPERTIES LANGUAGE CXX)
add_library(lua ${luafiles})
target_include_directories(lua PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
