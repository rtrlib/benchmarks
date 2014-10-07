#!/bin/bash

dir="pfx_roas"
count="10000 100000 1000000"
for ((i=0; i < 1000; i++)); do
    for j in $count; do
        if python gen-prefixes6.py $j 48 48 > roas; then
            echo "generated $j prefixes"
            ./load_bench_secpaper 10 roas |egrep "cpu|usecs" >> load-bench6-$j-48-48.log
        else
            echo "generation $j prefixes failed"
        fi
    done
done
