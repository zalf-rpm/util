project(Util-climate-climate)

message(STATUS "-> Util-climate-climate")

if(NOT TARGET db_lib)
	message(STATUS "target: db_lib")
	add_subdirectory("../../db" db)
endif()

if(NOT TARGET date_lib)
	message(STATUS "target: date_lib")
	add_subdirectory("../../tools/date" date)
endif()

if(NOT TARGET helpers_lib)
	message(STATUS "target: helpers_lib")
	add_subdirectory("../../tools/helpers" helpers)
endif()

if(NOT TARGET read_ini_lib)
	message(STATUS "target: read_ini_lib")
	add_subdirectory("../../tools/read_ini" read_ini)
endif()

if(NOT TARGET coord_trans_lib)
	message(STATUS "target: coord_trans_lib")
	add_subdirectory("../../tools/coord-trans" coord_trans)
endif()

if(NOT TARGET climate_common_lib)
	message(STATUS "target: climate_common_lib")
	add_subdirectory("../climate-common" climate_common)
endif()

add_library(climate_lib STATIC ../climate.h ../climate.cpp)
target_link_libraries(climate_lib PUBLIC climate_common_lib db_lib coord_trans_lib helpers_lib read_ini_lib date_lib)
target_include_directories(climate_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.. ../..)
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(climate_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

message(STATUS "<- Util-climate-climate")