#!/bin/bash         
	
##########################################################################################	
## Script for running spec2000 workloads on gem5, Based on the script I got from my previous supervisor, Prof. Hamed Farbeh ##
##########################################################################################

CACHE=--caches
L2=--l2cache
L2_SIZE=--l2_size=256kB
L2_ASSOC=--l2_assoc=16

L1D_SIZE=--l1d_size=32kB
L1D_ASSOC=--l1d_assoc=4

L1I_SIZE=--l1i_size=32kB
L1I_ASSOC=--l1i_assoc=4

LINE_SIZE=--cacheline_size=64

CPU_Type=--cpu-type=DerivO3CPU
BP_Type=--bp-type=Tournament2BP

LOG_DATA_DIR=~/project/stats
INPUT_DATA_DIR=~/project/HF_CPU2006/data
BINARY_DIR=~/project/HF_CPU2006/binaries/X86
GEM5_ROOT=~/project/gem5

MAX_INST=--maxinsts=10000000

OUT_EXT=.txt

#3.bzip2
echo "3.bzip2 Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.bzip2.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/bzip2_base.x86 \
	--options="$INPUT_DATA_DIR/bzip2/ref/input/control" 
echo "3.bzip2 Finished"
############################################################

#6.gamess
echo "6.gamess Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.gamess.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	--mem-size=2GB \-c $BINARY_DIR/gamess_base.x86 
	#NOTE: for 'hyperviscoplastic.inp' file you should not the postfix

echo "6.gamess Finished"
############################################################
#7.gcc
echo "7.gcc Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.gcc.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/gcc_base.x86 \
	--options="$INPUT_DATA_DIR/gcc/ref/input/200.in" 

echo "7.gcc Finished"

#9.gobmk
#NOTE: copy 'lib' and 'rules' folders in gem5 root
echo "9.gobmk Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.gobmk.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/gobmk_base.x86 \
	--option="$INPUT_DATA_DIR/gobmk/capture.tst"


echo "9.gobmk Finished"

############################################################
#11.h264ref
#NOTE: copy 'sss.yuv' into gem5 root. 
echo "11.h264ref Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.h264ref.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/h264ref_base.x86 \
	--options="-d $INPUT_DATA_DIR/h264ref/ref/input/sss_encoder_main.cfg" 

echo "11.h264ref Finished"
############################################################
#12.hmmer
echo "12.hmmer Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.hmmer.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/hmmer_base.x86 \
	--options="--fixed 0 --mean 500 --num 500000 --sd 350 --seed 0 \
	$INPUT_DATA_DIR/hmmer/ref/input/nph3.hmm" 


echo "12.hmmer Finished"
############################################################
#13.lbm
echo "13.lbm Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.lbm.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/lbm_base.x86 \
	--options="3000 lbm_reference.dat 0 0 $INPUT_DATA_DIR/lbm/ref/input/100_100_130_ldc.of" 

	#option is the content of 'lbm.in file'

echo "13.lbm Finished"
############################################################
#14.leslie3d
#NOTE: copy 'lib' and 'rules' folders in gem5 root
echo "14.leslie3d Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.leslie3d.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/leslie3d_base.x86 \
	--options="-I./lib $INPUT_DATA_DIR/perlbench/ref/input/checkspam.pl 2500 5 25 11 150 1 1 1 1" 


echo "14.leslie3d Finished"
############################################################
#15.libquantum
echo "13.462.libquantum Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.libquantum.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/libquantum_base.x86 \
	--options="1397 8" 

	#--options="$INPUT_DATA_DIR/libquantum/ref/input/control"
	#NOTE: we used the contents of 'control' file

echo "15.libquantum Finished"
##############################################################
#16.mcf
echo "16.mcf Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.mcf.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	--mem-size=2GB \-c $BINARY_DIR/mcf_base.x86 \
	--options="$INPUT_DATA_DIR/mcf/ref/input/inp.in" 


echo "16.mcf Finished"
############################################################
#17.milc
echo "17.milc Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.milc.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/milc_base.x86 \
	--input="$INPUT_DATA_DIR/milc/su3imp.in" 


echo "17.milc Finished"
############################################################
#18.namd
echo "18.namd Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour8.namd.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/namd_base.x86 \
	--options="--input $INPUT_DATA_DIR/namd/all/input/namd.input \
	--iterations 38 --output namd.out"


echo "18.namd Finished"
############################################################


echo "All Simulation Run Was Finished for All Configurations"