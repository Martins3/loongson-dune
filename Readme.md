# Process Virtualization

## Project Status
- MIPS arch finished and pass all the Linux Test Suite syscall tests. 😀
- Loongarch is under construction. 🚧
- X86 may be supported soon. 📅

You have a Loonson 3A4000 Computer, you can checkout to `mips-finished` tag and play with the code.

Any question, issue and email are welcome.

## Semantic different with native execution
1. segment fault
```c
  int * a= (int *)0x4000;

  *a = 12;
```

```c
[36403.260623] kvm [10252]: TLB ST fault:  cause 0x1080000c, status 0x740000a0, PC: 00000000bc0bbb16, BadVaddr: 0x4000
[36403.260626] kvm [10252]: Failed to find VMA for hva 0x4000
```

2. fd started at 6 instead of 3
4. `KVM_MAX_VCPU` limits the dune threads up to 16

## advantage over Standford Dune
1. Don't need kernel module.
2. Don't need to write code for intel and amd CPU separately.
3. Don't need to disable kernel kaslr.
4. Escape dune easily.
   - Process can enter dune and then escape dune whenever it wants to.
5. Nested Dune
6. Support multiple architectures.
7. Support fork related syscall, multi-thread program works almost perfectly.

## design explanation
When guest invoke syscall, it will be directed to hyerpcall and escape to host, then the syscall simulated in host userspace.
