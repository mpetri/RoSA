#!/bin/bash

threshold=4096
fac_dens=64
rosa_exec="../bin/rosa_sd2_delta"
tmp_dir="../tmp/"
outdir="../data/output/"
poccmin=50
poccmax=100

if [ $# -lt 1 ]; then
	echo "Usage: ${0} file [threshold] [fac_dens]" 
	echo "  file     : File containing the example string"
	echo "  threshold: Block threshold; default=${threshold}"
	echo "  fac_dens : Sampling parameter for factorization pointer; default=${fac_dens}"
	exit 1
fi

input=${1}

#if [ $# -gt 1 ]
#then
#	plen=${2}
#fi
#
#if [ $# -gt 2 ]
#then
#	poccmin=${3}
#fi

#if [ $# -gt 3 ]
#then
#	poccmax=${4}
#fi


${rosa_exec} --input_file=$input --threshold=4096 --fac_dens=1 --generate_patterns --pattern_len=4 --pattern_number=1000 --pattern_min_occ=$poccmin --pattern_max_occ=$poccmax --tmp_file_dir=$tmp_dir --output_dir=$outdir
${rosa_exec} --input_file=$input --threshold=4096 --fac_dens=1 --generate_patterns --pattern_len=10 --pattern_number=1000 --pattern_min_occ=$poccmin --pattern_max_occ=$poccmax --tmp_file_dir=$tmp_dir --output_dir=$outdir
${rosa_exec} --input_file=$input --threshold=4096 --fac_dens=1 --generate_patterns --pattern_len=20 --pattern_number=1000 --pattern_min_occ=$poccmin --pattern_max_occ=$poccmax --tmp_file_dir=$tmp_dir --output_dir=$outdir
${rosa_exec} --input_file=$input --threshold=4096 --fac_dens=1 --generate_patterns --pattern_len=40 --pattern_number=1000 --pattern_min_occ=$poccmin --pattern_max_occ=$poccmax --tmp_file_dir=$tmp_dir --output_dir=$outdir
${rosa_exec} --input_file=$input --threshold=4096 --fac_dens=1 --generate_patterns --pattern_len=100 --pattern_number=1000 --pattern_min_occ=$poccmin --pattern_max_occ=$poccmax --tmp_file_dir=$tmp_dir --output_dir=$outdir
