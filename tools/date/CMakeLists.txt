project(Util-tools-date)

message(STATUS "-> Util-tools-date")

if(NOT TARGET capnp_schemas_lib)
message(STATUS "target: capnp_schemas_lib")
	add_subdirectory("../../../mas-infrastructure/capnproto_schemas" mas-infrastructure/capnproto_schemas)
endif()

add_library(date_lib STATIC ../date.h ../date.cpp)
target_link_libraries(date_lib PUBLIC helpers_lib capnp_schemas_lib)
target_include_directories(date_lib PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/.. 
../.. 
${CMAKE_CURRENT_BINARY_DIR}/../../mas-infrastructure/capnproto_schemas
)
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(date_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

#add_dependencies(date_lib generate-date-capnp-h)

message(STATUS "date cbd: " ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "<- Util-tools-date")