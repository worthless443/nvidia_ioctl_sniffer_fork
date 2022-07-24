#include <fcntl.h>
#include <string.h>
#include <cstdlib>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <asm/ioctl.h>

#include<cstdio>
#include "tc_context.h"

#define NV_PLATFORM_MAX_IOCTL_SIZE 16384
#include "nv.h"
#include "nv_escape.h"
#include "nv-unix-nvos-params-wrappers.h"
#include "uvm_linux_ioctl.h"

#include<class/clc197.h>
#include <class/cl0080.h>  // NV01_DEVICE_0
#include<class/clc0b5.h>
#include <class/cl2080.h>  // NV20_SUBDEVICE_0
#include <class/clc461.h>  // TURING_USERMODE_A
#include <class/cl90f1.h>  // FERMI_VASPACE_A
#include <class/cla06c.h>  // KEPLER_CHANNEL_GROUP_A
#include <class/cl9067.h>  // FERMI_CONTEXT_SHARE_A
#include <class/clc56f.h>  // AMPERE_CHANNEL_GPFIFO_A
#include <class/clc7c0.h>  // AMPERE_COMPUTE_B
#include<class/clc097.h>

#include <ctrl/ctrla06c.h> // KEPLER_CHANNEL_GROUP_A
#include <ctrl/ctrlc36f.h> // VOLTA_CHANNELChannelGPFifoA

#define GENERIC_TYPE_0_A 0x4e
#define GENERIC_TYPE_1_A 0x38

// TODO: get this dynamically
// (you can get manually with 'nvidia-smi -L')
#define GPU_UUID "\xb4\xe9\x43\xc6\xdc\xb5\x96\x92\x6d\xb1\x04\x69\x18\x65\x8d\x08"


void test_assert(int fd_ctl, NvV32 hClass, NvHandle root, NvHandle parent, void *params) {
  int fd_dev  = open64("/dev/nvidia0", O_RDWR|O_CLOEXEC);
	  NVOS21_PARAMETERS p = {
    .hRoot = root, .hObjectParent = parent, .hClass = 0x44, .pAllocParms = params
  };
  NVOS32_PARAMETERS p32;
  NVOS54_PARAMETERS  p54;
  int ret0 = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_ALLOC,p),&p);
  int ret1 = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_VID_HEAP_CONTROL,p32),&p32);
  int ret2 = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_CONTROL,p54),&p54);
  assert(ret1==0);
  assert(ret2==0);
  assert(ret0==0);
  //hexdump(, 0x2);
}

static NvHandle alloc_object(int fd_ctl, NvV32 hClass, NvHandle root, NvHandle parent, void *params) {
  int fd_dev  = open64("/dev/nvidia0", O_RDWR|O_CLOEXEC);
  NVOS21_PARAMETERS p = {
    .hRoot = root, .hObjectParent = parent, .hClass = 0x013, .pAllocParms = params
  };
 NVOS32_PARAMETERS p0;

 NVOS21_PARAMETERS p1 =   {
 .hRoot = root, .hObjectParent = p.hObjectNew, .hClass = 0, .pAllocParms = params
 };
  NVOS32_PARAMETERS p62;
  NVOS47_PARAMETERS n47 = {
 	.hClient = parent, .hDevice = root
  };
  NV_MEMORY_ALLOCATION_PARAMS mAlloc;

  //int ret62 = ioctl(fd_ctl, _IOWR(IOC_IN|IOC_OUT|NV_IOCTL_MAGIC,NV_ESC_RM_VID_HEAP_CONTROL, p62), &p62);
  nv_ioctl_nvos33_parameters_with_fd *p2  = (nv_ioctl_nvos33_parameters_with_fd*)&p1;
  p2->fd  = fd_dev;
  int ret_mem = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_ALLOC,p), &p);
  //int ret_mem = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_VID_HEAP_CONTROL,mAlloc), &mAlloc);
  int ret = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_MAP_MEMORY, *p2), p2);
  int ret62 = ioctl(fd_ctl, _IOC(_IOC_READ,_IOC_TYPE(NVOS32_ALLOC_FLAGS_MAP_NOT_REQUIRED), GENERIC_TYPE_0_A ,GENERIC_TYPE_1_A), &n47);
  int ret0 = ioctl(fd_ctl, _IOWR(NVOS32_ALLOC_FLAGS_MAP_NOT_REQUIRED>>_IOC_READ, NV_ESC_RM_MAP_MEMORY, *p2), p2);//, &p1); // affter a lot of trial and error, right shifting works
  NVOS32_PARAMETERS *p3 = (NVOS32_PARAMETERS*)&p2->params;
  int ret1 = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_VID_HEAP_CONTROL,*p3), p3);//, &p1);

  assert(ret==0);
  assert(ret0==0);
  assert(ret1==0);
  assert(ret62==0);
  assert(ret_mem==0);

  if(ret62!=0) printf("\n%d\n", ret62);
  if(ret62!=0) printf("\n%d\n", n47.status);
  printf("\n%d\n", n47.status);
  //assert(ret == 0);
  //assert(p2->params.status==0);
	
  //NVOS21_PARAMETERS  
		

  return p.hObjectNew;
}

static void *mmap_object(int fd_ctl, NvHandle client, NvHandle device, NvHandle memory, NvU64 length, void *target, NvU32 flags) {
  int fd_dev0 = open64("/dev/nvidia0", O_RDWR | O_CLOEXEC);
  nv_ioctl_nvos33_parameters_with_fd p = {.params = {
    .hClient = client, .hDevice = device, .hMemory = memory, .length = length, .flags = flags
  }, .fd = fd_dev0 };

  NVOS32_PARAMETERS p32;
  int ret = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_MAP_MEMORY, p), &p);
  int ret1 = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_VID_HEAP_CONTROL, p32), &p32);
  assert(ret1==0);
  assert(ret == 0);
  assert(p.params.status == 0);
  return mmap64(target, length, PROT_READ|PROT_WRITE, MAP_SHARED | (target != NULL ? MAP_FIXED : 0), fd_dev0, 0);
}

static NvHandle heap_alloc(int fd_ctl, int fd_uvm, NvHandle root, NvHandle device, NvHandle subdevice, void *addr, NvU64 length, NvU32 flags, int mmap_flags, NvU32 type) {
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

static void rm_control(int fd_ctl, NvU32 cmd, NvHandle client, NvHandle object, void *params, NvU32 paramsize) {
  NVOS54_PARAMETERS p = {
    .hClient = client, .hObject = object, .cmd = cmd, .params = params, .paramsSize = paramsize
  };
  int ret = ioctl(fd_ctl, __NV_IOWR(NV_ESC_RM_CONTROL, p), &p);
  assert(ret == 0);
  assert(p.status == 0);
}
TcContext::TcContext() { init(); }
void TcContext::init() {
  init_device();
  return;
  init_uvm();
  init_mem();
  init_fifo();
}

void TcContext::init_device() {
  fd_ctl = open64("/dev/nvidiactl", O_RDWR | O_CLOEXEC);
  fd_uvm = open64("/dev/nvidia-uvm", O_RDWR | O_CLOEXEC);
  fd_dev0 = open64("/dev/nvidia0", O_RDWR | O_CLOEXEC);

  test_assert(fd_ctl, NV01_ROOT_CLIENT, 0, 0, NULL);
  return;
  root = alloc_object(fd_ctl, NV01_ROOT_CLIENT, 0, 0, NULL);
  NV0080_ALLOC_PARAMETERS ap0080 = { .hClientShare = root, .vaMode = NV_DEVICE_ALLOCATION_VAMODE_MULTIPLE_VASPACES };
  device = alloc_object(fd_ctl, NV01_DEVICE_0, root, root, &ap0080);
  
  //subdevice = alloc_object(fd_ctl, NV20_SUBDEVICE_0, root, device, NULL);
  subdevice = alloc_object(fd_ctl, NV20_SUBDEVICE_0, root, device, NULL);
  usermode = alloc_object(fd_ctl, PASCAL_A, root, subdevice, NULL);
  //gpu_mmio_ptr = mmap_object(fd_ctl, root, subdevice, usermode, 0x10000, NULL, 2);
  return;
  NV_VASPACE_ALLOCATION_PARAMETERS vap = {
    .flags = NV_VASPACE_ALLOCATION_FLAGS_ENABLE_PAGE_FAULTING | NV_VASPACE_ALLOCATION_FLAGS_IS_EXTERNALLY_OWNED,
    .vaBase = 0x60000
  };
  vaspace = alloc_object(fd_ctl, FERMI_VASPACE_A, root, device, &vap);
}

void TcContext::init_uvm() {
  {
    UVM_INITIALIZE_PARAMS p = {0};
    int ret = ioctl(fd_uvm, UVM_INITIALIZE, &p);
    assert(ret == 0);
    assert(p.rmStatus == 0);
  }

  {
    UVM_REGISTER_GPU_PARAMS p = {
      .rmCtrlFd = -1
    };
    memcpy(&p.gpu_uuid.uuid, GPU_UUID, 0x10);
    int ret = ioctl(fd_uvm, UVM_REGISTER_GPU, &p);
    assert(ret == 0);
    assert(p.rmStatus == 0);
  }

  {
    UVM_REGISTER_GPU_VASPACE_PARAMS p = {
      .rmCtrlFd = fd_ctl, .hClient = root, .hVaSpace = vaspace,
    };
    memcpy(&p.gpuUuid.uuid, GPU_UUID, 0x10);
    int ret = ioctl(fd_uvm, UVM_REGISTER_GPU_VASPACE, &p);
    assert(ret == 0);
    assert(p.rmStatus == 0);
  }
}

void TcContext::init_mem() {
  mem_handle = heap_alloc(fd_ctl, fd_uvm, root, device, subdevice,
    (void *)0x200400000, 0x200000,
    NVOS32_ALLOC_FLAGS_IGNORE_BANK_PLACEMENT | NVOS32_ALLOC_FLAGS_ALIGNMENT_FORCE |
    NVOS32_ALLOC_FLAGS_MEMORY_HANDLE_PROVIDED | NVOS32_ALLOC_FLAGS_MAP_NOT_REQUIRED | NVOS32_ALLOC_FLAGS_PERSISTENT_VIDMEM,
    0xc0000, NVOS32_TYPE_IMAGE);
  mem_error_handle = heap_alloc(fd_ctl, fd_uvm, root, device, subdevice,
    mem_error, 0x1000, 0xc001, 0, NVOS32_TYPE_NOTIFIER);
}

void TcContext::init_fifo() {
  NV_CHANNEL_GROUP_ALLOCATION_PARAMETERS cgap = {
    .engineType = NV2080_ENGINE_TYPE_GRAPHICS
  };
  channel_group = alloc_object(fd_ctl, KEPLER_CHANNEL_GROUP_A, root, device, &cgap);

  NV_CTXSHARE_ALLOCATION_PARAMETERS cap = {
    .hVASpace = vaspace,
    .flags = NV_CTXSHARE_ALLOCATION_FLAGS_SUBCONTEXT_ASYNC
  };
  share = alloc_object(fd_ctl, FERMI_CONTEXT_SHARE_A, root, channel_group, &cap);

  NV_CHANNELGPFIFO_ALLOCATION_PARAMETERS fifoap = {0};
  fifoap.hObjectError = mem_error_handle;
  fifoap.hObjectBuffer = mem_handle;
  fifoap.gpFifoOffset = 0x200400000;
  fifoap.gpFifoEntries = 0x400;
  fifoap.hContextShare = share;
  fifoap.hUserdMemory[0] = mem_handle;
  fifoap.userdOffset[0] = 0x2000;
  gpfifo = alloc_object(fd_ctl, AMPERE_CHANNEL_GPFIFO_A, root, channel_group, &fifoap);
  compute = alloc_object(fd_ctl, AMPERE_COMPUTE_B, root, gpfifo, NULL);
  // NOTE: the nvdriver also allocates a AMPERE_DMA_COPY_B here

  // this is the value you write to the doorbell register
  {
    NVC36F_CTRL_CMD_GPFIFO_GET_WORK_SUBMIT_TOKEN_PARAMS sp = {0};
    sp.workSubmitToken = -1;
    rm_control(fd_ctl, NVC36F_CTRL_CMD_GPFIFO_GET_WORK_SUBMIT_TOKEN, root, gpfifo, &sp, sizeof(sp));
    work_submit_token = sp.workSubmitToken;
    assert(work_submit_token != -1);
  }

  // register the FIFO with UVM
  {
    UVM_REGISTER_CHANNEL_PARAMS p = {0};
    memcpy(&p.gpuUuid.uuid, GPU_UUID, 0x10);
    p.rmCtrlFd = fd_ctl;
    p.hClient = root;
    p.hChannel = gpfifo;
    // TODO: is this right?
    p.base = 0x203600000;
    p.length = 0xf6e000;
    int ret = ioctl(fd_uvm, UVM_REGISTER_CHANNEL, &p);
    assert(ret == 0);
  }

  // enable the FIFO
  {
    NVA06C_CTRL_GPFIFO_SCHEDULE_PARAMS sp = {0};
    sp.bEnable = true;
    rm_control(fd_ctl, NVA06C_CTRL_CMD_GPFIFO_SCHEDULE, root, channel_group, &sp, sizeof(sp));
  }
}
