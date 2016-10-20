cmake_minimum_required (VERSION 2.6)
project (man_bomber)
set (man_bomber_VERSION_MAJOR 0)
set (man_bomber_VERSION_MINOR 1)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_C_FLAGS "-g")

configure_file (
	"${PROJECT_SOURCE_DIR}/src/include/man_bomber_config.h.in"
	"${PROJECT_SOURCE_DIR}/src/include/man_bomber_config.h"
	)

include_directories("${PROJECT_SOURCE_DIR}/src/include/")
add_executable(manbomber-srv src/srv.c src/srv-master.c src/srv-slave.c)
