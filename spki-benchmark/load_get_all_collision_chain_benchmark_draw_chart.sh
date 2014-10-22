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
mkdir -p $benchmark_result_dir

#Chart 1. - Line chart
mkdir -p $log_tmp_dir

echo "Collision chain length;Average CPU time consumption in µs to lookup one record" > $log_tmp_dir/log
echo "Data" >> $log_tmp_dir/log

#Run benchmark and save results
for (( i = $start_value; i <= $stop_value; i=$[$i+$increment_value] )); do
    cpu_time_in_microseconds=$(./src/load_get_all_benchmark $i 5000 100 | egrep "Average (.)* microseconds" | cut -d ' ' -f 2)
    cpu_time_in_microseconds=0$(echo "$cpu_time_in_microseconds / $i" | bc -l)
    echo $cpu_time_in_microseconds
    echo "$i;$cpu_time_in_microseconds" >> $log_tmp_dir/log
done

#Generate graph
./generate-line-chart.py "CPU time consumption of load_get_all_benchmark()" $log_tmp_dir/log $benchmark_result_dir/CPU_time_load_get_all_collision_chain_benchmark_$(start_value)_$(stop_value)_$(increment_value)_line_chart.pdf
echo ""
echo "Benchmark result is in ${benchmark_result_dir}/"