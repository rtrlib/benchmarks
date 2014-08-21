include_directories(/home/nils/git/rtrlib)
find_library(RTR_LIB rtr PATHS /home/nils/git/rtrlib)

find_package(Threads REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -Wformat-security -Wmissing-prototypes -Wmissing-declarations -Winit-self -Waggregate-return -Wmissing-format-attribute -Wundef -Wbad-function-cast -Wwrite-strings -Wformat=2")

add_executable(get_record get_record.c)
target_link_libraries(get_record ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(benchmark benchmark.c)
target_link_libraries(benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})
