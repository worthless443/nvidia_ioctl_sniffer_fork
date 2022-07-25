#!/bin/bash -e
mkdir -p out
ls out/libsniff.so 2>/dev/null || source ./make_sniff.sh
[ -e $LD_LIBRARY_PATH ] && export LD_LIBRARY_PATH="$(pwd)/out"
# Georgie's 
### 
 #ptxas simple.ptx --gpu-name sm_86 -o out/simple.o # -O1 -v
 #cuobjdump out/simple.o -sass
 
 clang++ gpu_driver.cc tc_context.cc \
   -L./out \
   -I./include \
   -I/opt/cuda/targets/x86_64-linux/include \
   -Iopen-gpu-kernel-modules/kernel-open/common/inc \
   -Iopen-gpu-kernel-modules/kernel-open/nvidia-uvm \
   -Iopen-gpu-kernel-modules/src/common/sdk/nvidia/inc \
   -Iopen-gpu-kernel-modules/src/nvidia/arch/nvalloc/unix/include \
   -Iopen-gpu-kernel-modules \
   -Iopen-gpu-kernel-modules/src/nvidia/generated -Iopen-gpu-kernel-modules/src/nvidia/inc/libraries -Iopen-gpu-kernel-modules/kernel-open/common/inc -I/usr/local/cuda/include -o out/gpu_driver -Iopen-gpu-kernel-modules -Iopen-gpu-kernel-modules/src/common/sdk/nvidia/inc -lpthread -lcuda -lsniff  /usr/lib64/libnvidia-ml.so
 #clang++ gpu_driver.cc -I/usr/local/cuda/include -o out/gpu_driver -Iopen-gpu-kernel-modules -Iopen-gpu-kernel-modules/src/common/sdk/nvidia/inc -lpthread
 #LD_PRELOAD=out/sniff.so out/gpu_driver
#export LD_LIBRARY_PATH="$(pwd)/out/"
#out/gpu_driver

#make -j16 && ./gpu_driver
