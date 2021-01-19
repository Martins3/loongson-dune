## snippet 

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


```
[ 2237.589777] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800008, PC: 00000000cbdb5148, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589779] kvm [12097]: TLB LD fault: cause 0x10800008, PC: 00000000cbdb5148, BadVaddr: 0x1200ba368
[ 2237.589784] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0x1200ba000 @ idx 1879
[ 2237.589827] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800008, PC: 00000000ccbbf423, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589829] kvm [12097]: TLB LD fault: cause 0x10800008, PC: 00000000ccbbf423, BadVaddr: 0x12000eec0
[ 2237.589834] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0x12000e000 @ idx 65
[ 2237.589837] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800004, PC: 0000000015907f9e, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589840] kvm [12097]: TLB MOD fault:  cause 0x10800004, status 0x540000a0, PC: 0000000015907f9e, BadVaddr: 0xfffbf39b40
[ 2237.589845] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0xfffbf38000 @ idx 807
[ 2237.589849] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800008, PC: 000000001766951b, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589851] kvm [12097]: TLB LD fault: cause 0x10800008, PC: 000000001766951b, BadVaddr: 0x1200208b0
[ 2237.589855] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0x120020000 @ idx 1860
[ 2237.589859] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800008, PC: 000000006affca30, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589861] kvm [12097]: TLB LD fault: cause 0x10800008, PC: 000000006affca30, BadVaddr: 0x120083820
[ 2237.589865] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0x120082000 @ idx 336
[ 2237.589868] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800008, PC: 0000000077d8e23d, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589871] kvm [12097]: TLB LD fault: cause 0x10800008, PC: 0000000077d8e23d, BadVaddr: 0x1200bc498 : _IO_stdfile_1_lock:
[ 2237.589874] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0x1200bc000 @ idx 1367
[ 2237.589877] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800004, PC: 0000000035f2f332, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589880] kvm [12097]: TLB MOD fault:  cause 0x10800004, status 0x540000a0, PC: 0000000035f2f332, BadVaddr: 0x1200bc490 : _IO_stdfile_1_lock
[ 2237.589883] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0x1200bc000 @ idx 1879
[ 2237.589886] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800008, PC: 00000000790bf850, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589888] kvm [12097]: TLB LD fault: cause 0x10800008, PC: 00000000790bf850, BadVaddr: 0x1200137b8 : _IO_new_file_xsputn:
[ 2237.589892] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0x120012000 @ idx 1346
[ 2237.589896] kvm [12097]: kvm_mips_handle_exit: cause: 0x9080000c, PC: 00000000b15c2204, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589898] kvm [12097]: TLB ST fault:  cause 0x9080000c, status 0x540000a0, PC: 00000000b15c2204, BadVaddr: 0x12c505734
[ 2237.589905] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0x12c504000 @ idx 992
[ 2237.589908] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800004, PC: 0000000011e70af2, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589911] kvm [12097]: TLB MOD fault:  cause 0x10800004, status 0x540000a0, PC: 0000000011e70af2, BadVaddr: 0x1200b8298 : _IO_2_1_stdout_:
[ 2237.589914] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0x1200b8000 @ idx 1367
[ 2237.589917] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800008, PC: 000000002012a56c, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589919] kvm [12097]: TLB LD fault: cause 0x10800008, PC: 000000002012a56c, BadVaddr: 0x1200154e0 __overflow:
[ 2237.589923] kvm [12097]: kvm_vz_host_tlb_inv: Invalidated root entryhi 0x120014000 @ idx 578
[ 2237.589927] kvm [12097]: kvm_mips_handle_exit: cause: 0x10800008, PC: 000000000d78f6f7, kvm_run: 0000000080e0cea0, kvm_vcpu: 000000005bc088e6
[ 2237.589929] kvm [12097]: TLB LD fault: cause 0x10800008, PC: 000000000d78f6f7, BadVaddr: 0xffffffff8900
[ 2237.589932] kvm [12097]: huxueshi : what's host_cp0_badinstr
[ 2237.589934] kvm [12097]: update_pc(): New PC: 0x120023a78
[ 2237.589941] kvm [12097]: huxueshi trace leave kvm_vz_vcpu_run:3570
```
