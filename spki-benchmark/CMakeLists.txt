add_executable(load_benchmark load_benchmark.c)
target_link_libraries(load_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(memory_benchmark memory_benchmark.c)
target_link_libraries(memory_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})
