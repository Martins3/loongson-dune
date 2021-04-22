# Loongson Dune

## Project status
- MIPS arch finished and pass all the Linux Test Suite syscall tests. 😀
- Loongarch is under construction. 🚧
- X86 may be supported soon. 📅

You have a Loonson 3A4000 Computer, you can checkout to `mips-finished` tag and play with the code.

Any question, issues and emails are welcomed.

## Semantic different with native execution
Process running in dune and host have some minor different behavior. 

1. segment fault.

Use following code as a example, in host, segment fault signal will send to the process, the process can register a handler for the signal. But in dune, kvm will `get_user_pages` failed and report the event to dune.
```c
  int * a= (int *)0x4000;

  *a = 12;
```

kernel log when accessing a unmapped area.
```c
[36403.260623] kvm [10252]: TLB ST fault:  cause 0x1080000c, status 0x740000a0, PC: 00000000bc0bbb16, BadVaddr: 0x4000
[36403.260626] kvm [10252]: Failed to find VMA for hva 0x4000
```

2. dune will consume 3 file descriptor for kvm(kvm_dev, vm, vcpu)
    1. fd started at 6 instead of 3.
    2. maximum fd one process can open would be less than expected.

3. `KVM_MAX_VCPU` limits the dune threads.

Loonson dune simulated one thread in the one vcpu, but kvm limits the vcpu number. So the program shouldn't create more than `KVM_MAX_VCPU` threads simultaneously.

## Advantage over Standford Dune
1. No kernel module.
2. Don't need to write code for intel and amd CPU separately.
3. Don't need to disable kernel kaslr.
4. Escape dune easily.
   - Process can enter dune and then escape dune whenever it wants to.
5. Nested Dune.
6. Support multiple architectures.
7. Support fork related syscall, multi-thread program works almost perfectly.
9. Less code, only about 2000 loc.
8. Much more stable


## Disadvantage
Syscall is emulated on host userspace instead of host kernel space. The user / kernel space switch is the overhead that Loonson introduce.

## Design explanation
1. In guest, gva mapped to gpa 1:1, so process "feels" the address space is same even jump into dune
2. When guest invoke syscall, it will be directed to hyerpcall and escape to host, then the syscall simulated in host userspace.

#### fork/clone
KVM disallow fork() and similar games when using a VM, so we should create another vm for child process when child doesn't share the VM.
```c
static long kvm_vcpu_ioctl(struct file *filp,
			   unsigned int ioctl, unsigned long arg)
{
	struct kvm_vcpu *vcpu = filp->private_data;
	void __user *argp = (void __user *)arg;
	int r;
	struct kvm_fpu *fpu = NULL;
	struct kvm_sregs *kvm_sregs = NULL;

	if (vcpu->kvm->mm != current->mm)
		return -EIO;
```
```diff
 History:        #0
 Commit:         6d4e4c4fca5be806b888d606894d914847e82d78
 Author:         Avi Kivity <avi@qumranet.com>
 Author Date:    Wed 21 Nov 2007 10:41:05 PM CST
 Committer Date: Wed 30 Jan 2008 11:53:13 PM CST

 KVM: Disallow fork() and similar games when using a VM

 We don't want the meaning of guest userspace changing under our feet.

 Signed-off-by: Avi Kivity <avi@qumranet.com>
```

- kvm_get_parent_thread_info
- emulate_fork
  - emulate_fork_by_two_vcpu
    - dup_vcpu
      - kvm_alloc_vcpu
    - **init_child_thread_info**
  - emulate_fork_by_two_vm
    - do_syscall
    - dup_vm
      - kvm_init_vm_with_one_cpu
      - **init_child_thread_info**

In `init_child_thread_info`, the child's gpr, especially the stack pointer, tls register and pc will be initilized.
