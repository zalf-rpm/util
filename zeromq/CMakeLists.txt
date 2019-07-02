project(Util-zeromq)

message(STATUS "-> Util-zeromq")

if(NOT TARGET date_lib)
	message(STATUS "target: date_lib")
	add_subdirectory("../tools/date" date)
endif()

if(NOT TARGET json11_lib)
	message(STATUS "target: json11_lib")
	add_subdirectory("../json11" json11)
endif()

find_package(ZeroMQ CONFIG REQUIRED)
#if(UNIX)
#	set(DL_LIB_NAME dl)
#endif()
add_library(zmq_lib STATIC zmq-helper.h zmq-helper.cpp zmq.hpp zhelpers.hpp)
target_link_libraries(zmq_lib PUBLIC libzmq-static json11_lib date_lib)
target_include_directories(zmq_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(MSVC AND MT_RUNTIME_LIB)
	target_compile_options(zmq_lib PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif()

message(STATUS "<- Util-zeromq")