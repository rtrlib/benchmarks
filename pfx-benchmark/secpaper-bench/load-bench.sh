#!/bin/bash

mkdir -p logs
count="1000 10000 100000 1000000 10000000"
for ((i=0; i < 1000; i++)); do
    for j in $count; do
        echo "Generating $j 24-24 prefixes ($i/1000)"
        python gen-prefixes.py $j 24 24 > roas
        for ((k=0; k < 10; k++)); do
            ./load_bench_secpaper 1 roas |egrep "cpu|usecs" >> logs/load-bench-$j-24-24.log
        done

        echo "Generating $j 8-31 prefixes ($i/1000)"
        python gen-prefixes.py $j 8 31 > roas
        for ((k=0; k < 10; k++)); do
            ./load_bench_secpaper 1 roas |egrep "cpu|usecs" >> logs/load-bench-$j-8-31.log
        done

        echo "Generating $j IPv6 48-48 prefixes ($i/1000)"
        python gen-prefixes6.py $j 48 48 > roas
        for ((k=0; k < 10; k++)); do
        ./load_bench_secpaper 1 roas |egrep "cpu|usecs" >> logs/load-bench6-$j-48-48.log
        done
    done
done

