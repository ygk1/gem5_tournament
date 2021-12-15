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
BP_Type=--bp-type=Tournament3BP

LOG_DATA_DIR=~/project/stats
INPUT_DATA_DIR=~/project/HF_CPU2006/data
BINARY_DIR=~/project/HF_CPU2006/binaries/X86
GEM5_ROOT=~/project/gem5

MAX_INST=--maxinsts=10000000

OUT_EXT=.txt

#cpu2006

#3.bzip2
echo "3.bzip2 Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour64bzip2.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/bzip2_base.x86 \
	--options="$INPUT_DATA_DIR/bzip2/ref/input/control"

echo "3.bzip2 Finished"
############################################################
#4.cactusADM
echo "4.cactusADM Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour64cactusADM.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	--mem-size=2GB \-c $BINARY_DIR/cactusADM_base.x86 \
	--options="$INPUT_DATA_DIR/cactusADM/ref/input/benchADM.par"

echo "4.cactusADM Finished"
############################################################

#7.gcc
echo "7.gcc Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour64gcc.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/gcc_base.x86 \
	--options="$INPUT_DATA_DIR/gcc/ref/input/200.in"

echo "7.gcc Finished"
############################################################

#9.gobmk
#NOTE: copy 'lib' and 'rules' folders in gem5 root
echo "9.gobmk Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour64gobmk.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/gobmk_base.x86 \
	--option="$INPUT_DATA_DIR/gobmk/capture.tst"

echo "9.gobmk Finished"
############################################################

############################################################
#15.libquantum
echo "13.462.libquantum Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour64libquantum.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/libquantum_base.x86 \
	--options="1397 8"
	#--options="$INPUT_DATA_DIR/libquantum/ref/input/control"
	#NOTE: we used the contents of 'control' file

echo "15.libquantum Finished"
##############################################################

#19.omnetpp
echo "19.omnetpp Started"
#NOTE: copy 'omnetpp.ini' in root directory
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour64omnetpp.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/omnetpp_base.x86 \
	#--options="$INPUT_DATA_DIR/omnetpp/ref/input/omnetpp.ini"
	#no need for option. just copy input in root

echo "19.omnetpp Finished"

#21.povray
echo "21.povray Started"
./build/X86/gem5.opt --stats-file="$LOG_DATA_DIR/log.tour64povray.txt" \
	configs/example/se.py $CPU_Type $BP_Type $MAX_INST $CACHE $L2 \
	$L1D_SIZE $L1I_SIZE $L1I_ASSOC $L1D_ASSOC $LINE_SIZE $L2_SIZE \
	\-c $BINARY_DIR/povray_base.x86 \
	--option="$INPUT_DATA_DIR/povray/SPEC-benchmark-test.ini"

echo "21.povray Finished"
############################################################


echo "All Simulation Run Was Finished for All Configurations"