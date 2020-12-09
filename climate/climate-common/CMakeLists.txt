project(Util-climate-climate_common)

message(STATUS "-> Util-climate-climate_common")

if(NOT TARGET date_lib)
  message(STATUS "target: date_lib")
	add_subdirectory(../../tools/date date)
endif()

add_library(climate_common_lib STATIC ../climate-common.h ../climate-common.cpp)
target_link_libraries(climate_common_lib PUBLIC date_lib)
target_include_directories(climate_common_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.. ../.. ${CMAKE_CURRENT_BINARY_DIR}/../../mas-infrastructure/capnproto_schemas)
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(climate_common_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

message(STATUS "<- Util-climate-climate_common")
