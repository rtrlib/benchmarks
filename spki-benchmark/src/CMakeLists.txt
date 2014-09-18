add_executable(load_add_entry_benchmark load_add_entry_benchmark.c)
target_link_libraries(load_add_entry_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(load_get_all_benchmark load_get_all_benchmark.c)
target_link_libraries(load_get_all_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(memory_benchmark memory_benchmark.c)
target_link_libraries(memory_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

# Memory benchmark target
add_custom_target(
	run_spki_memory_benchmark
	COMMAND cd .. && ./memory_benchmark_draw_graph.sh
	COMMAND echo ${test}
	COMMENT "Measure the memory consumption"
	DEPENDS memory_benchmark
)

# Memory benchmark target
add_custom_target(
	run_spki_load_add_entry_benchmark
	COMMAND cd .. && ./load_add_records_benchmark_draw_graph.sh
	COMMAND echo ${test}
	COMMENT "Measure cpu time while adding records"
	DEPENDS load_add_entry_benchmark
)