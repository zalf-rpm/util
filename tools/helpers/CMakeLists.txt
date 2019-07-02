project(Util-tools-helpers)

message(STATUS "-> Util-tools-helpers")
add_library(helpers_lib STATIC ../helper.h ../helper.cpp ../algorithms.h ../algorithms.cpp ../datastructures.h)
target_include_directories(helpers_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.. ../..)
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(helpers_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

message(STATUS "<- Util-tools-helpers")
