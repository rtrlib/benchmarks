add_executable(load_add_entry_benchmark load_add_entry_benchmark.c)
target_link_libraries(load_add_entry_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(load_get_all_benchmark load_get_all_benchmark.c)
target_link_libraries(load_get_all_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(memory_benchmark memory_benchmark.c)
target_link_libraries(memory_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

# Memory benchmark target
add_custom_target(
	run_spki_memory_benchmark
	COMMAND cd .. && ./memory_benchmark_draw_chart.sh
	COMMENT "Measure the memory consumption"
	DEPENDS memory_benchmark
)

# Load add_records benchmark target
add_custom_target(
	run_spki_load_add_entry_benchmark
	COMMAND cd .. && ./load_add_records_benchmark_draw_chart.sh
	COMMENT "Benchmark add_records"
	DEPENDS load_add_entry_benchmark
)

# Load get_all target
add_custom_target(
	run_spki_load_get_all_benchmark
	COMMAND cd .. && ./load_get_all_benchmark_draw_chart.sh
	COMMENT "Benchmark get_all"
	DEPENDS load_get_all_benchmark
)

# Run all SPKI benchmarks
add_custom_target(
	run_all_spki_benchmarks
	DEPENDS run_spki_load_add_entry_benchmark run_spki_memory_benchmark run_spki_load_get_all_benchmark
)