#!/bin/bash
#Benchmark the cpu load if records while adding records to the SPKI table.

log_tmp_dir=tmp
benchmark_result_dir=$(pwd)/result

if [[ "$#" != "3" ]]; then
    echo "Usage: $0 start stop step"
    exit
fi

start_value=$1 #Number of records added during first measurement.
stop_value=$2 #Number of records added during last measurement.
increment_value=$3 #Every measurement "number of records" will be increased by this value.

rm -rf $log_tmp_dir

mkdir -p $log_tmp_dir
mkdir -p $benchmark_result_dir

#Add captions [x-axis/y-axis]
echo "Number of added records;CPU time in s" > $log_tmp_dir/log
echo "Data" >> $log_tmp_dir/log

#Run benchmark and save results
for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
	#Start benchmark -> add $i records and repeat the measurment 5 times
	cpu_time_in_s=$(./src/load_add_entry_benchmark $i 10 | grep "Average" | cut -d ' ' -f 2)
    echo $cpu_time_in_s
	echo "$i;$cpu_time_in_s" >> $log_tmp_dir/log
done

#Generate graph
./generate-line-chart.py "CPU time consumption of spki_table_add_entry()" $log_tmp_dir/log $benchmark_result_dir/CPU_time_add_entry_${start_value}_${stop_value}_${increment_value}_line_chart.pdf
echo ""
echo "Add record benchmark result is in ${benchmark_result_dir}/"

rm -rf $log_tmp_dir
