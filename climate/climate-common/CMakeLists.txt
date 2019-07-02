project(Util-climate-climate_common)

message(STATUS "-> Util-climate-climate_common")

add_library(climate_common_lib STATIC ../climate-common.h ../climate-common.cpp)
target_include_directories(climate_common_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.. ../..)
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(climate_common_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

message(STATUS "<- Util-climate-climate_common")
