#!/bin/bash
#Benchmark the memory consumption of the SPKI table while adding records.

log_tmp_dir=tmp
benchmark_result_dir=$(pwd)/result

#Change this values
start_value=1000 #Number of records added during first measurement.
stop_value=100000 #Number of records added during last measurement.
increment_value=5000 #Every measurement "number of records" will be increased by this value.

rm -rf $log_tmp_dir
rm -rf $benchmark_result_dir

mkdir $log_tmp_dir
mkdir $benchmark_result_dir

#Add captions, change this if you like other captions...[x-axis/y-axis]
echo "Number of added records;Memory usage in MiB" > $log_tmp_dir/log

#Run benchmark and save results
for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
	#Start benchmark -> add $i records and repeat the measurment 5 times
	memory_usage_in_byte=$(./src/memory_benchmark $i 5 | grep "Average RSS" -A 2 | grep "MiB" | cut -d ' ' -f 1)
	echo "$i;$memory_usage_in_byte" >> $log_tmp_dir/log
done

#Generate graph
./generate-graph.py "Data" $log_tmp_dir/log $benchmark_result_dir/Memory\ usage\ $stop_value\ records.pdf
echo ""
echo "Memory benchmark result is in ${benchmark_result_dir}/"

#Clean up
#rm -R $log_tmp_dir