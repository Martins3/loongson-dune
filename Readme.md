# Process Virtualization

## GVA -> GPA is simpler in MIPS 
firstly, HVA is range is [0, 1<<40]
```c
void test_address_range(){
	if(access_ok(VERIFY_WRITE, 0, ((unsigned long long)1 << 40) - 1)){
    pr_debug("YES, we can\n");
  }else{
    pr_debug("No, we can't\n");
  }
}
```

```c
void test_virtual_address(){
	struct task_struct *g;
	rcu_read_lock();
	for_each_process (g) { 
    if(g->mm)
      pr_debug("%s ---> %lx %lx\n", g->comm, g->mm->mmap_base, g->mm->start_stack);
    else 
      pr_debug("%s doesn't have mm\n", g->comm);
	}
	rcu_read_unlock();
}
```
Secondly, loonson support 48 bit physical addres[^1]

## Semantic difference

1. segment fault
```c
  int * a= (int *)0x4000;

  *a = 12;
```

```c
[36403.260623] kvm [10252]: TLB ST fault:  cause 0x1080000c, status 0x740000a0, PC: 00000000bc0bbb16, BadVaddr: 0x4000
[36403.260626] kvm [10252]: Failed to find VMA for hva 0x4000
```

## advantage compared to Standford Dune
1. we can create kvm virtual machine in dune process
2. No need to write code for intel and amd CPU separately

## Source code explanation
1. code is writen in x86 machine and compiled and executed in CPU,
`dune/kvm.h` and `dune/mipskvm.h` is useless, but code editor running on x86 CPU will compliant
if it can't find the symbol of MIPS KVM.

[^1]: 用芯探核:基于龙芯的 Linux 内核探索解析 P58
