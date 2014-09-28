#!/bin/bash
#Benchmark the cpu time during look up

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


#Chart 1. - Bar chart

#Add group captions
echo "Number of lookups;CPU time in s" > $log_tmp_dir/log
group_names="${start_value}"
for (( i = $[$start_value+increment_value]; i <= $stop_value; i=$[$i+$increment_value] )); do
	group_names="${group_names};${i}"
done
echo $group_names >> $log_tmp_dir/log

#Add bar names for the legend
bar_names=""
for index in 0 1 3 5 10 50 80
do
	bar_names=$bar_names";Hash collision chance ${index}%"
done
echo ${bar_names:1} >> $log_tmp_dir/log

for index in 0 1 3 5 10 50 80 
do
	values=""
	for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
		#Start benchmark -> add $i records and repeat the measurment 5 times
		cpu_time_in_s=$(./src/load_get_all_benchmark $i 5 $index | grep "Average" | cut -d ' ' -f 2)
		echo $cpu_time_in_s
		values="${values};${cpu_time_in_s}"
	done
	echo -n "${values:1}" >> $log_tmp_dir/log
	echo "" >> $log_tmp_dir/log
done

#Generate graph
./generate-bar-chart.py "CPU time consumption of spki_table_get_all()" $log_tmp_dir/log $benchmark_result_dir/CPU_time_spki_table_get_all_bar_chart.pdf
echo "Benchmark result is in ${benchmark_result_dir}/"
rm -rf $log_tmp_dir

#Chart 2. - Line chart
mkdir -p $log_tmp_dir

echo "Number of added records;CPU time in s" > $log_tmp_dir/log
echo "Data" >> $log_tmp_dir/log

#Run benchmark and save results
for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
	cpu_time_in_s=$(./src/load_get_all_benchmark $i 5 $index | grep "Average" | cut -d ' ' -f 2)
    echo $cpu_time_in_s
	echo "$i;$cpu_time_in_s" >> $log_tmp_dir/log
done

#Generate graph
./generate-line-chart.py "CPU time consumption of load_get_all_benchmark()" $log_tmp_dir/log $benchmark_result_dir/CPU_time_load_get_all_benchmark_line_chart.pdf
echo ""
echo "Benchmark result is in ${benchmark_result_dir}/"