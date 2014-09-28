#!/bin/bash
#Benchmark the memory consumption of the SPKI table while adding records.

log_tmp_dir=tmp
benchmark_result_dir=$(pwd)/result

if [[ "$#" != "3" ]]; then
    echo "Usage: $0 start stop step"
    exit
fi

start_value=$1 #Number of records added during first measurement.
stop_value=$2 #Number of records added during last measurement.
increment_value=$3 #Every measurement "number of records" will be increased by this value

rm -rf $log_tmp_dir

mkdir -p $log_tmp_dir
mkdir -p $benchmark_result_dir

#Add captions [x-axis/y-axis]
echo "Number of added records;Memory usage in MiB" > $log_tmp_dir/log
echo "Data" >> $log_tmp_dir/log

#Run benchmark and save results
for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
	#Start benchmark -> add $i records and repeat the measurment 5 times
	memory_usage_in_byte=$(./src/memory_benchmark $i 10 | grep "Average RSS" -A 2 | grep "MiB" | cut -d ' ' -f 1)
	echo $memory_usage_in_byte
    echo "$i;$memory_usage_in_byte" >> $log_tmp_dir/log
done

#Generate graph
./generate-line-chart.py "Memory consumption of SPKI table" $log_tmp_dir/log $benchmark_result_dir/Memory_usage_add_entry.pdf
echo ""
echo "Memory benchmark result is in ${benchmark_result_dir}/"

rm -rf $log_tmp_dir
