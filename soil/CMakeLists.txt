project(Util-soil)

message(STATUS "-> Util-soil")

if(NOT TARGET helpers_lib)
message(STATUS "target: helpers_lib")
	add_subdirectory("../tools/helpers" helpers)
endif()

if(NOT TARGET capnp_schemas_lib)
message(STATUS "target: capnp_schemas_lib")
	add_subdirectory("../../mas-infrastructure/capnproto_schemas" mas-infrastructure/capnproto_schemas)
endif()

add_library(soil_lib STATIC constants.h constants.cpp conversion.h conversion.cpp soil.h soil.cpp)
target_link_libraries(soil_lib PUBLIC helpers_lib capnp_schemas_lib)
target_include_directories(soil_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} .. ${CMAKE_CURRENT_BINARY_DIR}/../../mas-infrastructure/capnproto_schemas)
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(soil_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

message(STATUS "<- Util-soil")