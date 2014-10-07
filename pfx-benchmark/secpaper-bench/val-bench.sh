#!/bin/bash

dir="data"
mkdir -p $dir

for ((i=0; i<500; i++)); do
    for prefix in $(ls -1 $dir/|sed -r 's/-(invalid|valid|notfound).*//g'|sort\
    |uniq); do
        roa_file="$dir/$prefix-roas.txt"
        for state in invalid notfound valid; do
            val_file="$dir/$prefix-$state.txt"
            if [ -s "$roa_file" -a -s $val_file ]; then
                ./benchmark 20 "$roa_file" "$val_file"\
                                logs/"$prefix-$state.log"
            fi
        done
    done
done
