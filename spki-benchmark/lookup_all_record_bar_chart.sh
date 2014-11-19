#!/bin/bash
#Benchmark the cpu time during look up

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
log_dir=$(pwd)/log
benchmark_name=get_all_records_${start_value}_${stop_value}_${increment_value}_bar_chart
log_file=$log_dir/$benchmark_name.txt

mkdir -p $log_dir
mkdir -p $benchmark_result_dir

#Chart 1. Measure how long it takes to lookup X records in a SPKI table which contains X records.

#Add group captions
echo "Number of lookups;CPU time in s" > $log_file
group_names="${start_value}"
for (( i = $[$start_value+increment_value]; i <= $stop_value; i=$[$i+$increment_value] )); do
    group_names="${group_names};${i}"
done
echo $group_names >> $log_file

#Add bar names for the legend
bar_names=""
for index in 0 1 3 5 10 50 80
do
    bar_names=$bar_names";Hash collision chance ${index}%"
done
echo ${bar_names:1} >> $log_file

for index in 0 1 3 5 10 50 80 
do
    values=""
    for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
        #Start benchmark -> add $i records and repeat the measurment 5 times
        cpu_time_in_s=$(./src/load_get_all_benchmark $i $num_of_passes $index | egrep "Average .* seconds" | cut -d ' ' -f 2)
        echo $cpu_time_in_s
        values="${values};${cpu_time_in_s}"
    done
    echo -n "${values:1}" >> $log_file
    echo "" >> $log_file
done

#Generate graph
./generate-bar-chart.py "CPU time consumption: Bulk look up with collisions" $log_file $benchmark_result_dir/$benchmark_name.pdf
echo "Benchmark result is in ${benchmark_result_dir}/"
