project(Util-tools-date)

message(STATUS "-> Util-tools-date")

add_library(date_lib STATIC ../date.h ../date.cpp)
target_include_directories(date_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.. ../..)
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(date_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

message(STATUS "<- Util-tools-date")