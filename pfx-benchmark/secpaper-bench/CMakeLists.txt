add_executable(benchmark benchmark.c)
target_link_libraries(benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(load_bench load_bench.c)
target_link_libraries(load_bench ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})
