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
8. Much more stable

## Disadvantage
Syscall is emulated on host userspace instead of host kernel space. The user / kernel space switch is the overhead that Loonson introduce.

## Design explanation
When guest invoke syscall, it will be directed to hyerpcall and escape to host, then the syscall simulated in host userspace.
