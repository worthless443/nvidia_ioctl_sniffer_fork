#include<signal.h>
#include "helpers.h"
#include "nouveau.h"
#include "nvml.h"


#include "kernel-open/nvidia-uvm/uvm_linux_ioctl.h"
#include "kernel-open/nvidia-uvm/uvm_ioctl.h"

#define NV_PLATFORM_MAX_IOCTL_SIZE 0xFFF
#include "kernel-open/common/inc/nv-ioctl-numbers.h"
#include "kernel-open/common/inc/nv.h"
#include "src/common/sdk/nvidia/inc/nvos.h"

#include "src/nvidia/generated/g_allclasses.h"
#include "src/nvidia/arch/nvalloc/unix/include/nv_escape.h"
#include "src/nvidia/arch/nvalloc/unix/include/nv-unix-nvos-params-wrappers.h"

#include "src/common/sdk/nvidia/inc/class/cl0080.h"
#include "src/common/sdk/nvidia/inc/class/cl2080.h"

#include "src/common/sdk/nvidia/inc/ctrl/ctrl0000/ctrl0000gpu.h"
#include "src/common/sdk/nvidia/inc/ctrl/ctrlc36f.h"
#include "src/common/sdk/nvidia/inc/ctrl/ctrla06c.h"
#include "src/common/sdk/nvidia/inc/ctrl/ctrla06f/ctrla06fgpfifo.h"
#include "rs.h"

#include <thread>
#include <sys/ioctl.h>
#include <fcntl.h>
#ifndef DISABLE_CUDA_SUPPORT
#include <cuda.h>
#endif
#include <unistd.h>
#include <sys/mman.h>

#include "tc_context.h"

//void gpu_memset(int subc, void *)

void gpu_setup(struct nouveau_pushbuf *push) {
  /*BEGIN_NVC0(push, 1, NVC6C0_SET_OBJECT, 1);
  PUSH_DATA(push, AMPERE_COMPUTE_B);
  BEGIN_NVC0(push, 1, NVC6C0_NO_OPERATION, 1);
  PUSH_DATA(push, 0);*/

  BEGIN_NVC0(push, 1, NVC6C0_SET_SHADER_SHARED_MEMORY_WINDOW_A, 2); PUSH_DATAhl(push, 0x00007FFFF4000000);
  BEGIN_NVC0(push, 1, NVC6C0_SET_SHADER_LOCAL_MEMORY_NON_THROTTLED_A, 2); PUSH_DATAhl(push, 0x004B0000);

  //BEGIN_NVC0(push, 1, NVC6C0_SET_SHADER_LOCAL_MEMORY_A, 1); PUSH_DATAh(push, 0x00007FFFC0000000);
  //BEGIN_NVC0(push, 1, NVC6C0_SET_SHADER_LOCAL_MEMORY_B, 1); PUSH_DATAl(push, 0x00007FFFC0000000);

  //BEGIN_NVC0(push, 1, NVC6C0_SET_SHADER_LOCAL_MEMORY_WINDOW_A, 1); PUSH_DATAh(push, 0x00007FFFF2000000);
  //BEGIN_NVC0(push, 1, NVC6C0_SET_SHADER_LOCAL_MEMORY_WINDOW_B, 1); PUSH_DATAl(push, 0x00007FFFF2000000);

  /*BEGIN_NVC0(push, 1, NVC6C0_SET_SPA_VERSION, 1);
  PUSH_DATAl(push, 0x00000806);
  BEGIN_NVC0(push, 1, NVC6C0_SET_CWD_REF_COUNTER, 1);
  PUSH_DATAl(push, 0x000F0000);
  BEGIN_NVC0(push, 1, NVC6C0_SET_RESERVED_SW_METHOD07, 1);
  PUSH_DATAl(push, 1);
  BEGIN_NVC0(push, 1, NVC6C0_SET_VALID_SPAN_OVERFLOW_AREA_A, 3);
  PUSH_DATAl(push, 2);
  PUSH_DATAl(push, 0);
  PUSH_DATAl(push, 0x001E0000);
  BEGIN_NVC0(push, 1, NVC6C0_SET_SHADER_LOCAL_MEMORY_NON_THROTTLED_C, 1);
  PUSH_DATAl(push, 0x28);*/
}

// NVC6B5 = AMPERE_DMA_COPY_A
void gpu_dma_copy(struct nouveau_pushbuf *push, uint64_t dst, uint64_t src, int len) {
  BEGIN_NVC0(push, 4, NVC6B5_OFFSET_IN_UPPER, 4);
  PUSH_DATAhl(push, src);
  PUSH_DATAhl(push, dst);  // NVC6B5_OFFSET_OUT_UPPER
  BEGIN_NVC0(push, 4, NVC6B5_LINE_LENGTH_IN, 1);
  PUSH_DATA(push, len);
  BEGIN_NVC0(push, 4, NVC6B5_LAUNCH_DMA, 1);
  // 0x100 = NVC6B5_LAUNCH_DMA_DST_MEMORY_LAYOUT_PITCH
  // 0x 80 = NVC6B5_LAUNCH_DMA_SRC_MEMORY_LAYOUT_PITCH
  // 0x  2 = NVC6B5_LAUNCH_DMA_DATA_TRANSFER_TYPE_NON_PIPELINED
  PUSH_DATA(push, 0x00000182);
}

// NVC6C0 = AMPERE_COMPUTE_A
void gpu_memcpy(struct nouveau_pushbuf *push, uint64_t dst, const uint32_t *dat, int len) {
  assert(len%4 == 0);

  BEGIN_NVC0(push, 1, NVC6C0_OFFSET_OUT_UPPER, 2);
  PUSH_DATAhl(push, dst);
  BEGIN_NVC0(push, 1, NVC6C0_LINE_LENGTH_IN, 2);
  PUSH_DATA(push, len);
  PUSH_DATA(push, 1);    // NVC6C0_LINE_COUNT
  BEGIN_NVC0(push, 1, NVC6C0_LAUNCH_DMA, 1);
  // 0x 40 = NVC6C0_LAUNCH_DMA_SYSMEMBAR_DISABLE_TRUE
  // 0x  1 = NVC6C0_LAUNCH_DMA_DST_MEMORY_LAYOUT_PITCH
  PUSH_DATA(push, 0x41);

  int words = len/4;
  BEGIN_NIC0(push, 1, NVC6C0_LOAD_INLINE_DATA, words);
  for (int i = 0; i < words; i++) {
    PUSH_DATA(push, dat[i]);
  }
}

#if static_cast<bool>(-1)

#endif
void gpu_compute(struct nouveau_pushbuf *push, uint64_t qmd, uint64_t program_address, uint64_t constant_address, int constant_length) {
  BEGIN_NVC0(push, 1, NVC6C0_SET_INLINE_QMD_ADDRESS_A, 2);
  PUSH_DATAhl(push, qmd>>8);

  uint32_t dat[0x40];
  memset(dat, 0, sizeof(dat));
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_QMD_GROUP_ID,,, 0x3F, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_SM_GLOBAL_CACHING_ENABLE,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_INVALIDATE_TEXTURE_HEADER_CACHE,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_INVALIDATE_TEXTURE_SAMPLER_CACHE,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_INVALIDATE_TEXTURE_DATA_CACHE,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_INVALIDATE_SHADER_DATA_CACHE,,, 1, dat);

  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CWD_MEMBAR_TYPE,,, NVC6C0_QMDV03_00_CWD_MEMBAR_TYPE_L1_SYSMEMBAR, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_API_VISIBLE_CALL_LIMIT,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_SAMPLER_INDEX,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_SHARED_MEMORY_SIZE,,, 0x400, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_MIN_SM_CONFIG_SHARED_MEM_SIZE,,, 3, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_MAX_SM_CONFIG_SHARED_MEM_SIZE,,, 0x1A, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_QMD_MAJOR_VERSION,,, 3, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_REGISTER_COUNT_V,,, 0x10, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_TARGET_SM_CONFIG_SHARED_MEM_SIZE,,, 3, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_BARRIER_COUNT,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_SHADER_LOCAL_MEMORY_HIGH_SIZE,,, 0x640, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_PROGRAM_PREFETCH_SIZE,,, 0xa, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_SASS_VERSION,,, 0x86, dat);

  // group
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CTA_RASTER_WIDTH,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CTA_RASTER_HEIGHT,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CTA_RASTER_DEPTH,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CTA_THREAD_DIMENSION0,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CTA_THREAD_DIMENSION1,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CTA_THREAD_DIMENSION2,,, 1, dat);

  // this isn't needed, what does it do?
  /*FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_RELEASE0_ADDRESS_LOWER,,, release_address, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_RELEASE0_ADDRESS_UPPER,,, release_address>>32, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_RELEASE0_ENABLE,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_RELEASE0_STRUCTURE_SIZE,,, 1, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_RELEASE0_PAYLOAD_LOWER,,, 6, dat);*/

  // program
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_PROGRAM_ADDRESS_LOWER,,, program_address, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_PROGRAM_ADDRESS_UPPER,,, program_address>>32, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_PROGRAM_PREFETCH_ADDR_LOWER_SHIFTED,,, program_address>>8, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_PROGRAM_PREFETCH_ADDR_UPPER_SHIFTED,,, program_address>>40, dat);

  // args
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CONSTANT_BUFFER_ADDR_UPPER(0),,, constant_address>>32, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CONSTANT_BUFFER_ADDR_LOWER(0),,, constant_address, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CONSTANT_BUFFER_SIZE_SHIFTED4(0),,, constant_length, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CONSTANT_BUFFER_INVALIDATE(0),,, NVC6C0_QMDV03_00_CONSTANT_BUFFER_INVALIDATE_TRUE, dat);
  FLD_SET_DRF_NUM_MW(C6C0_QMDV03_00_CONSTANT_BUFFER_VALID(0),,, NVC6C0_QMDV03_00_CONSTANT_BUFFER_VALID_TRUE, dat);

  BEGIN_NVC0(push, 1, NVC6C0_LOAD_INLINE_QMD_DATA(0), 0x40);
  for (int i = 0; i < 0x40; i++) {
    PUSH_DATA(push, dat[i]);
  }
}

void kick(volatile uint32_t *doorbell, int cb_index) {
  // is this the doorbell register?
  //printf("kick\n");
  *doorbell = cb_index;
}

uint64_t trivial[] = {
  0x00005a00ff057624, 0x000fe200078e00ff,  // IMAD.MOV.U32 R5, RZ, RZ, c[0x0][0x168]
  0x0000580000027a02, 0x000fe20000000f00,  // MOV R2, c[0x0][0x160]
  0x0000590000037a02, 0x000fca0000000f00,  // MOV R3, c[0x0][0x164]
  0x0000000502007986, 0x000fe2000c101904,  // STG.E [R2.64], R5
  0x000000000000794d, 0x000fea0003800000,  // EXIT
};

#define GPU_UUID "\xb4\xe9\x43\xc6\xdc\xb5\x96\x92\x6d\xb1\x04\x69\x18\x65\x8d\x08"

NvHandle alloc_object(int fd_ctl, NvV32 hClass, NvHandle root, NvHandle parent, void *params) {
  NVOS21_PARAMETERS p = {
    .hRoot = root, .hObjectParent = parent, .hClass = hClass, .pAllocParms = params
  };
  int ret = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_ALLOC, p), &p);
  assert(ret == 0);
  assert(p.status == 0);
  return p.hObjectNew;
}

void *mmap_object(int fd_ctl, NvHandle client, NvHandle device, NvHandle memory, NvU64 length, void *target, NvU32 flags) {
  int fd_dev0 = open64("/dev/nvidia0", O_RDWR | O_CLOEXEC);
  nv_ioctl_nvos33_parameters_with_fd p = {.params = {
    .hClient = client, .hDevice = device, .hMemory = memory, .length = length, .flags = flags
  }, .fd = fd_dev0 };
  int ret = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_MAP_MEMORY, p), &p);
  assert(ret == 0);
  assert(p.params.status == 0);
  return mmap64(target, length, PROT_READ|PROT_WRITE, MAP_SHARED | (target != NULL ? MAP_FIXED : 0), fd_dev0, 0);
}

NvHandle heap_alloc(int fd_ctl, int fd_uvm, NvHandle root, NvHandle device, NvHandle subdevice, void *addr, NvU64 length, NvU32 flags, int mmap_flags, NvU32 type) {
  NVOS32_PARAMETERS p = {
    .hRoot = root, .hObjectParent = device, .function = NVOS32_FUNCTION_ALLOC_SIZE,
    .data = { .AllocSize = {
      .owner = root, .type = type,
      .flags = flags, .size = length
    } }
  };
  int ret = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_VID_HEAP_CONTROL, p), &p);
  assert(p.status == 0);
  NvHandle mem = p.data.AllocSize.hMemory;
  void *local_ptr = mmap_object(fd_ctl, root, subdevice, mem, length, addr, mmap_flags);
  assert(local_ptr == (void *)addr);

  if (type == 0) {
    UVM_CREATE_EXTERNAL_RANGE_PARAMS p = {0};
    p.base = (NvU64)local_ptr;
    p.length = length;
    int ret = ioctl(fd_uvm, UVM_CREATE_EXTERNAL_RANGE, &p);
    assert(ret == 0);
    assert(p.rmStatus == 0);
  }

  if (type == 0) {
    UVM_MAP_EXTERNAL_ALLOCATION_PARAMS p = {0};
    p.base = (NvU64)local_ptr;
    p.length = length;
    p.rmCtrlFd = fd_ctl;
    p.hClient = root;
    p.hMemory = mem;
    p.gpuAttributesCount = 1;
    memcpy(&p.perGpuAttributes[0].gpuUuid, GPU_UUID, 0x10);
    p.perGpuAttributes[0].gpuMappingType = 1;
    int ret = ioctl(fd_uvm, UVM_MAP_EXTERNAL_ALLOCATION, &p);
    assert(ret == 0);
    assert(p.rmStatus == 0);
  }
  return mem;
}

void rm_control(int fd_ctl, NvU32 cmd, NvHandle client, NvHandle object, void *params, NvU32 paramsize) {
  NVOS54_PARAMETERS p = {
    .hClient = client, .hObject = object, .cmd = cmd, .params = params, .paramsSize = paramsize
  };
  int ret = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_CONTROL, p), &p);
  assert(ret == 0);
  assert(p.status == 0);
}
void setpush(struct nouveau_pushbuf *push, int *value) {
	for(int i=0;;i++) { 
	  int **addr_store = NULL;
	  int **addr = (int**)malloc(sizeof(uint32_t*)*sizeof(*push));
	  if(i==1) addr_store = addr;
	  if(((addr - addr_store)/8)==(sizeof(*push)/sizeof(uint32_t*))-1) break;
	  addr = (int**)push + i;
	  *addr = value;
	}
}
struct nvml_s {
	int ret_succ;
	int gpu_is_lost;
	int sharelib_succ;
};

struct nvml_s getUUID() {
	nvmlDevice_t nDevice;
  	nvmlReturn_t nvmlret = nvmlDeviceGetUUID(nDevice, (char*)"\x03\x45\x95", 2);
	struct nvml_s nvmls = { 
		 .ret_succ = NVML_SUCCESS, .gpu_is_lost=NVML_ERROR_GPU_IS_LOST,.sharelib_succ = NVML_ERROR_LIBRARY_NOT_FOUND 
	};
	return nvmls;
}

void *copy_nov(int *addr, uint32_t base, uint32_t offset) {
	struct nouveau_pushbuf *gpu_addr = (struct nouveau_pushbuf*)(base+offset);
  	setpush(gpu_addr,addr);
	return (void*)(base+offset);
}
// BLOCK_IOCTL=79,80,84,98,11,12,78,85,73,82,16,20,30,13,15,17,19,35,71 EXIT_IOCTL=106 NVDRIVER=1 ./driver.sh 
int main(int argc, char *argv[]) {
  void *mem_error = (void*)0x7ffff7ffb000;
  int work_submit_token = 0;
  void *gpu_mmio_ptr = (void *)0x13370000;

//  if (getenv("NVDRIVER")) {
//	  printf("getting gpu driver");
//    TcContext ctx;
//    ctx.init();
//    mem_error = ctx.mem_error;
//    work_submit_token = ctx.work_submit_token;
//    gpu_mmio_ptr = ctx.gpu_mmio_ptr;
//  } else {
//	  //return 1;
//  }
  TcContext ctx;
  return 0;
  ctx.init();
  //cuInit(0);
  printf("\n");
    // our GPU driver doesn't support init. use CUDA
    // TODO: remove linking to CUDA
//#ifndef DISABLE_CUDA_SUPPORT
//cuInit(0);
    CUdevice pdev;
    //CUcontext pctx;
//    printf("**** init\n");
      //cuInit(0);
//    printf("**** device\n");
    cuDeviceGet(&pdev, 0);
//    printf("**** ctx\n");
//    //kcuCtxCreate(&pctx, 0, pdev);
//    // NOTE: this can be wrong
//    work_submit_token = 5;
//#endif
//  }
//
//  printf("**************** INIT DONE ****************\n");
  //clear_gpu_ctrl();
  uint64_t gpu_base = 0x600000;
//
//  // set up command queue
  uint64_t cmdq = gpu_base+0x6000;
  struct nouveau_pushbuf push_local; //  { .cur = (uint32_t*)0x5c000002 };
  struct nouveau_pushbuf *push  = &push_local;
  int *addr = (int*)mmap((int*)gpu_base, sizeof(int*), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON,0,0);
  *addr = 69;
  setpush(push, addr);

  struct nouveau_pushbuf *test = (struct nouveau_pushbuf*)copy_nov(addr, gpu_base, 0x90);
  struct nouveau_pushbuf *addr_ = (struct nouveau_pushbuf*)((int*)(gpu_base + 0x90));
  ///gpu_setup(push);
  gpu_memcpy(push, gpu_base+0x40, (const uint32_t*)"\xaa\xbb\xcc\xdd", 400);
  int *push_addr = (int*)(gpu_base+0x90);
  //kprintf("%d\n", **(int**)(gpu_base + 0x90));
  //memcpy((int*)gpu_base+0x40, push, sizeof(*push));
  //printf("%d\n", *((int*)((struct nouveau_pushbuf*)(gpu_base+0x40))-1));
  //printf("%lx\n", push->cur);
  //struct nouveau_pushbuf *push = &push_local;
//
  struct nouveau_pushbuf push_local1; //= { .cur = (uint32_t*)cmdq };
  //push = &push_local1;
//
//  // load program
//  uint32_t program[0x100];
//  if (getenv("TRIVIAL")) {
//    printf("running trivial program\n");
//    memcpy(program, trivial, sizeof(trivial));
//  } else {
//    FILE *f = fopen("out/simple.o", "rb");
//    fseek(f, 0x600, SEEK_SET);
//    fread(program, 1, 0x180, f);
//    fclose(f);
//    printf("loaded program\n");
//  }
//
//
  //gpu_setup(push);

  //printf("%d\n", *(int*)0x65);
  push = &push_local;

  struct {
    uint64_t addr;
    uint32_t value;
  } args;

  args.addr = gpu_base;
  args.value = 0x1337;
  gpu_compute(push, gpu_base+0x90, gpu_base+0x80, gpu_base+0x20, 0x160+sizeof(args));
  return 0;
// load program and args
  //gpu_memcpy(push, gpu_base+0x1000, (const uint32_t*)"\x90\x90", 0x180);
  return 0;
  //gpu_memcpy(push, gpu_base+0x2160, (const uint32_t*)&args, 0x10);
//  printf("memcpyed program into gpu memory\n");
//
//  // run program
//
//  // do this too
//  gpu_dma_copy(push, gpu_base+0x14, gpu_base+0, 8);
//
//  // kick off command queue
//  uint64_t sz = (uint64_t)push->cur - cmdq;
//  /**((uint64_t*)0x2004003f0) = cmdq | (sz << 40) | 0x20000000000;
//  *((uint64_t*)0x20040208c) = 0x7f;*/
//  *((uint64_t*)0x200400000) = cmdq | (sz << 40) | 0x20000000000;
//  *((uint64_t*)0x20040208c) = 1;
//  kick(&((volatile uint32_t*)gpu_mmio_ptr)[0x90/4], work_submit_token);
//
//  // wait for it to run
//  volatile uint32_t *done = (uint32_t*)0x200402088;
//  printf("ran to queue %d\n", *done);
//  int cnt = 0; while (!*done && cnt<1000) { usleep(1000); cnt++; }
//  usleep(10*1000);
//  printf("ran to queue %d\n", *done);
//
//  // dump ram to check
//  printf("pc %p\n", (void*)gpu_base);
//  hexdump((void*)gpu_base, 0x20);
//  printf("error\n");
//
//  // NvNotification
//  // ROBUST_CHANNEL_GR_EXCEPTION = 0xd
//  // ROBUST_CHANNEL_FIFO_ERROR_MMU_ERR_FLT = 0x1f
//  // ROBUST_CHANNEL_GR_CLASS_ERROR = 0x45
//  hexdump((void*)mem_error, 0x40);
//
//  if (getenv("HANG")) { while(1) sleep(1); }
}
