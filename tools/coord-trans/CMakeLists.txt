project(Util-tools-coord_trans)

message(STATUS "-> Util-tools-coord_trans")

if(NOT TARGET db_lib)
	message(STATUS "target: db_lib")
	add_subdirectory("../../db" db)
endif()

if(NOT TARGET climate_common_lib)
	message(STATUS "target: climate_common_lib")
	add_subdirectory("../../climate/climate-common" climate_common)
endif()

if(NOT TARGET helpers_lib)
	message(STATUS "target: helpers-lib")
	add_subdirectory("../helpers" helpers)
endif()

find_package(PROJ4 CONFIG REQUIRED)
add_library(coord_trans_lib STATIC ../coord-trans.h ../coord-trans.cpp)
target_link_libraries(coord_trans_lib PUBLIC climate_common_lib proj db_lib helpers_lib)
target_include_directories(coord_trans_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.. ../.. ${PROJ4_INCLUDE_DIRS})
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(coord_trans_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

message(STATUS "<- Util-tools-coord_trans")