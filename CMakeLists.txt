cmake_minimum_required (VERSION 2.6)
project (man_bomber)
set (man_bomber_VERSION_MAJOR 0)
set (man_bomber_VERSION_MINOR 1)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

enable_testing()

configure_file (
	"${PROJECT_SOURCE_DIR}/src/include/man_bomber_config.h.in"
	"${PROJECT_SOURCE_DIR}/src/include/man_bomber_config.h"
	)

set(CMAKE_C_FLAGS_DEBUGE "-Wall -Wextra -g")

include_directories("${PROJECT_SOURCE_DIR}/src/include/")
add_executable(manbomber-srv src/srv/srv.c src/srv/srv-master.c
	src/srv/srv-slave.c src/lib/magic.c src/lib/recv-data.c src/lib/list.c)
add_executable(srv_data_recv tests/srv_data_recv.c)
add_executable(list tests/list.c src/lib/list.c)

add_test(srv_data_recv ${EXECUTABLE_OUTPUT_PATH}/srv_data_recv)
add_test(list ${EXECUTABLE_OUTPUT_PATH}/list)

option(HOGE_FUGA "Return fuga\n to hoge\r\n" OFF)
if(HOGE_FUGA)
	add_definitions(-DENABLE_HOGE_FUGA)
endif()
