project(Util-tools-debug)

message(STATUS "-> Util-tools-debug")
add_library(debug_lib STATIC ../debug.h ../debug.cpp)
target_include_directories(debug_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(debug_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

message(STATUS "<- Util-tools-debug")
