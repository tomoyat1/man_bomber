cmake_minimum_required (VERSION 3.0)
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
add_executable(manbomber-srv src/srv/srv.c src/srv/srv-util.c src/srv/srv-master.c
	src/srv/srv-slave.c src/lib/magic.c src/lib/recv-data.c src/lib/send-data.c src/lib/list.c)
add_executable(manbomber-cli src/cli/cli.c src/cli/cli_recv.c
	src/cli/cli_send.c src/graphics/mainDisplay.c src/graphics/result.c
	src/graphics/title.c src/lib/magic.c src/lib/recv-data.c
	src/lib/send-data.c)
add_executable(srv_data_recv tests/srv_data_recv.c src/lib/recv-data.c
	src/lib/magic.c)
add_executable(list tests/list.c src/lib/list.c)
add_executable(bomb_equality tests/srv_bomb_equality.c src/srv/srv-master.c
	src/srv/srv-util.c src/lib/list.c)

target_link_libraries(manbomber-srv rt)
target_link_libraries(manbomber-cli ncursesw)
target_link_libraries(bomb_equality rt)

target_compile_options(manbomber-cli PRIVATE -std=c99)

add_test(srv_data_recv ${EXECUTABLE_OUTPUT_PATH}/srv_data_recv)
add_test(list ${EXECUTABLE_OUTPUT_PATH}/list)
add_test(bomb_equality ${EXECUTABLE_OUTPUT_PATH}/bomb_equality)

option(HOGE_FUGA "Return fuga\n to hoge\r\n" OFF)
if(HOGE_FUGA)
	add_definitions(-DENABLE_HOGE_FUGA)
endif()
