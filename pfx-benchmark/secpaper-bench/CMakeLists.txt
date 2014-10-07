add_executable(benchmark benchmark.c)
target_link_libraries(benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(load_bench load_bench.c)
target_link_libraries(load_bench ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

#Copy scripts in build tree
add_custom_target(
    copy_pfx_secpaper_scripts ALL
    COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/*.sh ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/*.py ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Copy script files to build tree"
)