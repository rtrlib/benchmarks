#!/bin/bash
if [[ "$#" != "4" ]]; then
    echo "Usage: $0 start stop step number_of_passes"
    exit
fi

start_value=$1 #Number of records added during first measurement.
stop_value=$2 #Number of records added during last measurement.
increment_value=$3 #Every measurement "number of records" will be increased by this value
num_of_passes=$4

benchmark_result_dir=$(pwd)/result
benchmark_name=Memory_usage_benchmark_${start_value}_${stop_value}_${increment_value}
benchmark_result_dir=$(pwd)/result
log_dir=$(pwd)/log
log_file=$log_dir/$benchmark_name.txt

mkdir -p $log_dir
mkdir -p $benchmark_result_dir

#Add captions [x-axis/y-axis]
echo "Number of added records;Memory usage in MiB" > $log_file
echo "Data" >> $log_file

#Run benchmark and save results
for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
	#Start benchmark -> add $i records and repeat the measurment 5 times
	memory_usage_in_byte=$(./src/memory_benchmark $i $num_of_passes | grep "Average RSS" -A 2 | grep "MiB" | cut -d ' ' -f 1)
	echo $memory_usage_in_byte
    echo "$i;$memory_usage_in_byte" >> $log_file
done

#Generate graph
./generate-line-chart.py "Memory consumption of SPKI table" $log_file $benchmark_result_dir/$benchmark_name.pdf
echo ""
echo "Memory benchmark result is in ${benchmark_result_dir}/"
