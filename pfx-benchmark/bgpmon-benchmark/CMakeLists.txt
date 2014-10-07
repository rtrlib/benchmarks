add_executable(benchmark_bgpmon benchmark.c)
target_link_libraries(benchmark_bgpmon ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

#Copy scripts in build tree
add_custom_target(
    copy_pfx_bgpmon_scripts ALL
    COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/*.py ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Copy script files to build tree"
)
