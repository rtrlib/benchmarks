add_executable(load_add_entry_benchmark load_add_entry_benchmark.c spki_test_data.c)
target_link_libraries(load_add_entry_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(load_get_all_benchmark load_get_all_benchmark.c spki_test_data.c)
target_link_libraries(load_get_all_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

add_executable(memory_benchmark memory_benchmark.c spki_test_data.c)
target_link_libraries(memory_benchmark ${RTR_LIB} ${CMAKE_THREAD_LIBS_INIT})

# Memory benchmark target
add_custom_target(
	run_spki_memory_benchmark
	COMMAND cd .. && ./memory_benchmark_line_chart.sh 0 2000000 100000 10
	COMMENT "Measure memory consumption of SPKI table"
	DEPENDS memory_benchmark
)

# Load add_records benchmark target
add_custom_target(
	run_spki_add_entry_benchmark
	COMMAND cd .. && ./add_records_line_chart.sh 0 2000000 100000 5
	DEPENDS load_add_entry_benchmark
)

# Load get_all target
add_custom_target(
	run_spki_lookup_all_benchmark
	COMMAND cd .. && ./lookup_all_records_line_chart.sh 0 2000000 250000 20
	COMMAND cd .. && ./lookup_all_record_bar_chart.sh  200000 1000000 200000 5
	DEPENDS load_get_all_benchmark
)

add_custom_target(
	run_spki_get_singel_record_benchmark
	COMMAND cd .. && ./lookup_single_record_line_charts.sh 0 2000000 100000 15
	DEPENDS load_get_all_benchmark
)

# Load get_all indirection
add_custom_target(
	run_spki_collision_benchmark
	COMMAND cd .. && ./collision_chain_line_chart.sh 0 5000 250 30
	DEPENDS load_get_all_benchmark
)

# Run all SPKI benchmarks
add_custom_target(
	run_all_spki_benchmarks
	DEPENDS run_spki_memory_benchmark run_spki_add_entry_benchmark run_spki_lookup_all_benchmark run_spki_get_singel_record_benchmark run_spki_collision_benchmark
)

