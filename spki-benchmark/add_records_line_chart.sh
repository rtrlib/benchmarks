#!/bin/bash
if [[ "$#" != "4" ]]; then
    echo "Usage: $0 start stop step number_of_passes"
    exit
fi

start_value=$1 #Number of records added during first measurement.
stop_value=$2 #Number of records added during last measurement.
increment_value=$3 #Every measurement "number of records" will be increased by this value
num_of_passes=$4

start_value=$1 #Number of records added during first measurement.
stop_value=$2 #Number of records added during last measurement.
increment_value=$3 #Every measurement "number of records" will be increased by this value.

benchmark_name=add_records_benchmark_${start_value}_${stop_value}_${increment_value}_line_chart
benchmark_result_dir=$(pwd)/result
log_dir=$(pwd)/log
log_file=$log_dir/$benchmark_name.txt

mkdir -p $log_dir
mkdir -p $benchmark_result_dir

#Chart 1. Measure how long it takes to add X records in an empty SPKI table

#Add captions [x-axis/y-axis]
echo "Number of added records;CPU time in s" > $log_file
echo "Data" >> $log_file

#Run benchmark and save results
for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
	#Start benchmark -> add $i records and repeat the measurment 5 times
	cpu_time_in_s=$(./src/load_add_entry_benchmark $i $num_of_passes | grep "Average" | cut -d ' ' -f 2)
    echo $cpu_time_in_s
	echo "$i;$cpu_time_in_s" >> $log_file
done

#Generate graph
./generate-line-chart.py "CPU time consumption of spki_table_add_entry()" $log_file $benchmark_result_dir/$benchmark_name.pdf
echo ""
echo "Add record benchmark result is in ${benchmark_result_dir}/"

