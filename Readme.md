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
