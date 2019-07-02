project(Util-climate-climate_file_io)

message(STATUS "-> Util-climate-climate_file_io")

if(NOT TARGET json11_lib)
	message(STATUS "target: json11_lib")
	add_subdirectory("../../json11" json11)
endif()

if(NOT TARGET climate_common_lib)
	message(STATUS "target: climate_common_lib")
	add_subdirectory("../climate-common" climate_common)
endif()

add_library(climate_file_io_lib STATIC ../climate-file-io.h ../climate-file-io.cpp)
target_link_libraries(climate_file_io_lib PUBLIC climate_common_lib json11_lib)
target_include_directories(climate_file_io_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.. ../..)
message(STATUS "MT_RUNTIME_LIB: " ${MT_RUNTIME_LIB})
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(climate_file_io_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()
#set_target_properties(libclimateio PROPERTIES PREFIX "")
#set_target_properties(libclimateio PROPERTIES COMPILE_DEFINITIONS "USE_DLL;DLL_EXPORTS")

message(STATUS "<- Util-climate-climate_file_io")