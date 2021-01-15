# TODO

- 学会写 MIPS 汇编
- kvmtool 的测试
- kvm 的 log 需要打开，进而重新编译一下


- memslot 的构造, 如此，在下一个 tlb 的 miss 才可以
// TODO 如果物理地址的范围，可以映射多个相同大小的方位
// 1. 调查 mmap_base 和 stack_base 的是什么 ?
// 2. 物理限制大小

- [ ] 调查 kvm 的回收机制是什么 ?

- [ ] TLB miss 是不是总是是 invalid 的状态的

## code

1. access_ok
```c
void test_address_range(){
	if(access_ok(VERIFY_WRITE, 0, ((unsigned long long)1 << 40) - 1)){
    pr_debug("YES, we can\n");
  }else{
    pr_debug("No, we can't\n");
  }
}
```

2. 40bit virtual address space
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


## bug
- [ ] kvm 总是首先会访问 物理地址为 0 的位置，无论是 guest.pc 如何设置 ?

```
[53817.455376] kvm [11318]: gpm_mm.pgd @ 98000004520f8000
[53817.456065] kvm [11318]: kvm @ 9800000452dc1000: create cpu 0 at 980000045504e000
[53817.456107] kvm [11318]: start 9800000455470080 end 98000004554700c0 handler 9800000455472000
[53817.456112] kvm [11318]: guest cop0 page @ 9800000455460000 gprs @ 980000045504e390 tlb @ 980000045504e950 pc @ 980000045504e4a0
[53817.456115] kvm [11318]: pending exception @ 980000045504e940
[53817.456117] kvm [11318]: fcr31 @ 980000045504e8a8
[53817.456119] kvm [11318]: count_bias @ 980000045504e91c period @ 980000045504e938
[53817.456121] kvm [11318]: vzguestid @ 980000045504f980
[53817.456123] kvm [11318]: exit_reason @ 9800000457f9c008
[53817.456125] kvm [11318]: run @ 9800000457f9c000
[53817.456127] kvm [11318]: wait @ 980000045504fab0
[53817.456129] kvm [11318]: stable timer @ 980000045504fa00
[53817.456131] kvm [11318]: use stable timer 1
[53817.456133] kvm [11318]:
```
- [ ] 日志中间的这些 9800000 很诡异

- [ ] 重新检查一遍 config
  - [ ] config 寄存器只是读去，无法使用的，所以不应该怀疑 config ?
- [ ] hypercall 指令无法使用 ?

**利用 ioctl 将寄存器的数值全部读去回来吧 !**
