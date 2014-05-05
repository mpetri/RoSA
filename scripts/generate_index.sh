#!/bin/bash

threshold=4096
fac_dens=64
rosa_exec="../bin/rosa_sd2_delta"
tmp_dir="../tmp/"
outdir="../data/output/"

if [ $# -lt 1 ]; then
	echo "Usage: ${0} file [threshold] [fac_dens]" 
	echo "  file     : File containing the example string"
	echo "  threshold: Block threshold; default=${threshold}"
	echo "  fac_dens : Sampling parameter for factorization pointer; default=${fac_dens}"
	exit 1
fi

input=${1}

#${rosa_exec} --input_file=$input --threshold=4096 --fac_dens=1 --generate_index --tmp_file_dir=$tmp_dir --output_dir=$outdir
#${rosa_exec} --input_file=$input --threshold=4096 --fac_dens=16 --generate_index --tmp_file_dir=$tmp_dir --output_dir=$outdir
${rosa_exec} --input_file=$input --threshold=4096 --fac_dens=256 --generate_index --tmp_file_dir=$tmp_dir --output_dir=$outdir

