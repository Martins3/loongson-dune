#include <linux/kvm.h>
#include <sys/mman.h>
#include <stddef.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define PROT_RW (PROT_READ | PROT_WRITE)
#define MAP_ANON_NORESERVE (MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE)

static inline void *alloc_hva(u64 size)
{
	return mmap(NULL, size, PROT_RW, MAP_ANON_NORESERVE, -1, 0);
}

static void report(const char *prefix, const char *err, va_list params)
{
	char msg[1024];
	vsnprintf(msg, sizeof(msg), err, params);
	fprintf(stderr, " %s%s\n", prefix, msg);
}

static void error_builtin(const char *err, va_list params)
{
	report(" Error: ", err, params);
}

int pr_err(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	error_builtin(err, params);
	va_end(params);
	return -1;
}

int kvm__init_ram(int vm_fd, u64 size, u64 guest_phys_addr, u64 userspace_addr)
{
	struct kvm_userspace_memory_region mem;
	int slot = 0;
	int flags = 0;
	int ret;

	mem = (struct kvm_userspace_memory_region){
		.slot = slot,
		.flags = flags,
		.guest_phys_addr = guest_phys_addr,
		.memory_size = size,
		.userspace_addr = userspace_addr,
	};

	ret = ioctl(vm_fd, KVM_SET_USER_MEMORY_REGION, &mem);
	if (ret < 0) {
    // TODO how errno works ?
		ret = -errno;
	}
	return ret;
}

int kvm__init_guest(){
  int ret = 0;

  // general

  return ret;
}


int kvm__init()
{
	char dev_path[] = "/dev/kvm";
	int ret;
	int kvm_fd = open(dev_path, O_RDWR);
	if (kvm_fd < 0) {
		pr_err("unable to open %s", dev_path);
		goto err;
	}

	ret = ioctl(kvm_fd, KVM_GET_API_VERSION, 0);
	if (ret != KVM_API_VERSION) {
		pr_err("KVM_GET_API_VERSION ioctl");
		goto err_sys_fd;
	}

// TODO what about it in MIPS ?
#define KVM_VM_TYPE 0

	int vm_fd = ioctl(kvm_fd, KVM_CREATE_VM, KVM_VM_TYPE);
	if (vm_fd < 0) {
		pr_err("KVM_CREATE_VM ioctl");
		ret = vm_fd;
		goto err_sys_fd;
	}

  u64 ram_size = 1024;
  u64 guest_phys_addr = 0;
  void * userspace_addr = alloc_hva(ram_size);
  ret = kvm__init_ram(vm_fd, ram_size, guest_phys_addr, (u64)userspace_addr);
  if (ret < 0){
    pr_err("KVM_SET_USER_MEMORY_REGION ioctl");
    goto err_vm_fd;
  }

// TODO maybe just exit, no need to close them
err_vm_fd:
	close(vm_fd);
err_sys_fd:
	close(kvm_fd);
err:
	return 1;
}
