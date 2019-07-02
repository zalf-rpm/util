project(Util-soil)

message(STATUS "-> Util-soil")

if(NOT TARGET helpers_lib)
message(STATUS "target: helpers_lib")
	add_subdirectory("../tools/helpers" helpers)
endif()

add_library(soil_lib STATIC constants.h constants.cpp conversion.h conversion.cpp soil.h soil.cpp)
target_link_libraries(json11_lib PUBLIC helpers_lib)
target_include_directories(soil_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ..)
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(soil_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

message(STATUS "<- Util-soil")