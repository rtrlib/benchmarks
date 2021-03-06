#!/bin/bash
if [[ "$#" != "4" ]]; then
    echo "Usage: $0 start stop step number_of_passes"
    exit
fi

start_value=$1 #Number of records added during first measurement.
stop_value=$2 #Number of records added during last measurement.
increment_value=$3 #Every measurement "number of records" will be increased by this value
num_of_passes=$4

benchmark_name=get_single_record_benchmark_${start_value}_${stop_value}_${increment_value}_line_chart
benchmark_result_dir=$(pwd)/result
log_dir=$(pwd)/log
log_file=$log_dir/$benchmark_name.txt

mkdir -p $log_dir
mkdir -p $benchmark_result_dir

#Chart 1. Measure how long it takes to lookup one record in a SPKI table which contains X records.
echo "Number of records;Average CPU time consumption in µs to look up one record" > $log_file
echo "Data" >> $log_file

#Run benchmark and save results
for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
    cpu_time_in_microseconds=$(./src/load_get_all_benchmark $i $num_of_passes 0 | egrep "Average (.)* microseconds" | cut -d ' ' -f 2)
    cpu_time_in_microseconds=0$(echo "$cpu_time_in_microseconds / $i" | bc -l)
    echo $cpu_time_in_microseconds
    echo "$i;$cpu_time_in_microseconds" >> $log_file
done

#Generate graph
./generate-line-chart.py "CPU time consumption: Single record look up" $log_file $benchmark_result_dir/$benchmark_name.pdf
echo ""
echo "Benchmark result is in ${benchmark_result_dir}/"
