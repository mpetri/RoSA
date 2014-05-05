#!/bin/bash

tmp_dir="../tmp/"
outdir="../data/output"
input="../data/input/web-4GB"
pat="../data/patterns/web-4GB.pattern-ng"

datestr=$(date +%Y-%m-%d-%H-%M-%S)
resfile="../data/results/all_factor_lens-$datestr.csv"
echo "B;;flen" > $resfile

for B in 4096 1024
do
    for R in 16
    do
        for bin in ../bin/rosa_sd2_delta
        do
            $bin --input_file=$input --threshold=$B --fac_dens=$R --output_factorlens --tmp_file_dir=$tmp_dir --output_dir=/devhome1/mpetri/RoSA/data/output/  >> $resfile
        done
    done
done

