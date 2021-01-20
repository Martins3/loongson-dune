## sync
```
rsync --delete -avzh --exclude='/.git' --filter="dir-merge,- .gitignore" maritns3@10.90.50.149:/home/maritns3/core/loongson-dune/3day/ /home/loongson/dune/
```

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
