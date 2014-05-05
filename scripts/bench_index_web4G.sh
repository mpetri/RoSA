#!/bin/bash

tmp_dir="../tmp/"
outdir="../data/output"
input="../data/input/web-4GB"
pat="../data/patterns/web-4GB.pattern-ng"

datestr=$(date +%Y-%m-%d-%H-%M-%S)
resfile="../data/results/match_stats-$datestr.csv"
echo "id;file;B;R;i;plen;method;count;len_total;len_accessed;decoded;num_factors;ids_decoded;match_time;count_time" > $resfile

for B in 4096
do
    for R in 16 256
    do
        for bin in ../bin/matchlz_*
        do
            $bin --input_file=$input --threshold=$B --fac_dens=$R --benchmark_matchlz --tmp_file_dir=$tmp_dir --output_dir=/devhome1/mpetri/RoSA/data/output/ --pattern_file=$pat >> $resfile
        done
    done
done

