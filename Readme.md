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
