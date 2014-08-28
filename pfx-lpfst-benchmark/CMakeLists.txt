add_executable(get_record get_record.c)
target_link_libraries(get_record ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(benchmark benchmark.c)
target_link_libraries(benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})
