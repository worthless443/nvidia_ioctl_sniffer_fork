TARGET=gpu_driver
BENCHMARK=benchmark/run.cc
LINK=-L./out
LIBS=-lcuda -lpthread  -ldl -lstdc++
SHARED_LIBRARY=out/libsniffas.so
INCLUDE=-I./include -I./\
  -I/opt/cuda/targets/x86_64-linux/include \
  -Iopen-gpu-kernel-modules/kernel-open/common/inc \
  -Iopen-gpu-kernel-modules/kernel-open/nvidia-uvm \
  -Iopen-gpu-kernel-modules/src/common/sdk/nvidia/inc \
  -Iopen-gpu-kernel-modules/src/nvidia/arch/nvalloc/unix/include \
  -Iopen-gpu-kernel-modules \
  -Iopen-gpu-kernel-modules/src/nvidia/generated -Iopen-gpu-kernel-modules/src/nvidia/inc/libraries -Iopen-gpu-kernel-modules/kernel-open/common/inc -I/usr/local/cuda/include \
  -Iopen-gpu-kernel-modules -Iopen-gpu-kernel-modules/src/common/sdk/nvidia/inc 

INCLUDE_SNIFF=-Iopen-gpu-kernel-modules -Iopen-gpu-kernel-modules/kernel-open/common/inc -Iopen-gpu-kernel-modules/src/common/sdk/nvidia/inc
CXX_FLAGS=-fno-exceptions -fPIC
Objs=gpu_driver.o tc_context.o
Obj=src/sniff.cc

all: $(SHARED_LIBRARY) $(TARGET) 

$(Objs): %.o : %.cc
	clang++ -c $(INCLUDE) src/$^ -o $@

$(TARGET) : $(Objs)
	clang++ $(LINK) $(LIBS) $(patsubst %.o,%.o,$(^)) /usr/lib64/libnvidia-ml.so -lsniffas -o $@

$(SHARED_LIBRARY):
	clang++ $(CXX_FLAGS) $(LIBS) $(INCLUDE) $(Obj) -shared -o $@
.PHONY: benchmark
benchmark:
	g++  $(BENCHMARK)  -o benchmark/bench
#TODO add variables for commands and benchmark log file
clean_obj:
	rm -rf *.o
clean: clean_obj
	rm -rf $(TARGET) $(SHARED_LIBRARY) 
clean_all:clean
	rm -rf benchmark/bench	

