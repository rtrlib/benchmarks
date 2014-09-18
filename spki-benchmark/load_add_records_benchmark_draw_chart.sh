#!/bin/bash
#Benchmark the cpu load if records while adding records to the SPKI table.

log_tmp_dir=tmp
benchmark_result_dir=$(pwd)/result

#Change this values
start_value=1000 #Number of records added during first measurement.
stop_value=100000 #Number of records added during last measurement.
increment_value=5000 #Every measurement "number of records" will be increased by this value.

rm -Rf $log_tmp_dir
rm -Rf $benchmark_result_dir

mkdir $log_tmp_dir
mkdir $benchmark_result_dir

#Add captions
echo "Number of added records;CPU time in s" > $log_tmp_dir/log

#Run benchmark and save results
for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
	#Start benchmark -> add $i records and repeat the measurment 5 times
	cpu_time_in_s=$(./src/load_add_entry_benchmark $i 5 | grep "Average" | cut -d ' ' -f 2)
	echo "$i;$cpu_time_in_s" >> $log_tmp_dir/log
done

#Generate graph
./generate-line-chart.py "Data" $log_tmp_dir/log $benchmark_result_dir/Add\ entry\ benchmark\ -\ $stop_value\ records.pdf


rm -R $log_tmp_dir