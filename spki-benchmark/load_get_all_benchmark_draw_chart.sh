#!/bin/bash
#Benchmark the cpu time during look up

log_tmp_dir=tmp
benchmark_result_dir=$(pwd)/result

#Change this values
start_value=10000 #Number of records added during first measurement.
stop_value=100000 #Number of records added during last measurement.
increment_value=10000 #Every measurement "number of records" will be increased by this value.

rm -Rf $log_tmp_dir
rm -Rf $benchmark_result_dir

mkdir $log_tmp_dir
mkdir $benchmark_result_dir


#Benchmark 1. Add $stop_value records and then measure the time it takes to lookup up all records.

#Add captions
echo "Number of lookups;CPU time in s" > $log_tmp_dir/log

# #Run benchmark and save result
# for (( j = 0; j <= 10; j++ )); do
# 	for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
# 		#Start benchmark -> add $i records and repeat the measurment 5 times
# 		cpu_time_in_s=$(./src/load_get_all_benchmark $i 10 $j | grep "Average" | cut -d ' ' -f 2)
# 		echo "$i;$cpu_time_in_s" >> $log_tmp_dir/log
# 	done
# 	#Generate graph
# 	./generate-line-chart.py "Data" $log_tmp_dir/log $benchmark_result_dir/get_all_from_${start_value}_to_${stop_value}_duplicate_chance_${j}%.pdf
# 	rm $log_tmp_dir/log
# done

rm -R $log_tmp_dir

