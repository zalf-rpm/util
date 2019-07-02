project(Util-db)

message(STATUS "-> Util-db")

if(NOT TARGET sqlite3_lib)
	message(STATUS "target: sqlite3_lib")
	add_subdirectory("sqlite3")
endif()
if(NOT TARGET db_lib)
	message(STATUS "target: db_lib")
	add_subdirectory("db")
endif()

message(STATUS "<- Util-db")
