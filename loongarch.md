# Loongarch 特别注意

## 某一个版本需要增加的补丁
在 commit d5497d72b0893d587dfe08b4162b9cb6e32e0555  tag: 4.19.167-5 的位置存在两个 bug 需要修复

```diff
diff --git a/arch/loongarch/kvm/loongisa.c b/arch/loongarch/kvm/loongisa.c
index 3fb74baf02c5..6e91cbf98de9 100644
--- a/arch/loongarch/kvm/loongisa.c
+++ b/arch/loongarch/kvm/loongisa.c
@@ -457,7 +457,7 @@ int kvm_arch_vcpu_ioctl_run(struct kvm_vcpu *vcpu, struct kvm_run *run)
                vcpu->mmio_needed = 0;
        } else if (vcpu->arch.is_hypcall) {
                /* set return value for hypercall v0 register */
-               vcpu->arch.gprs[2] = run->hypercall.ret;
+               vcpu->arch.gprs[4] = run->hypercall.ret;
                vcpu->arch.is_hypcall = 0;
        }


diff --git a/arch/loongarch/kvm/lsvz.c b/arch/loongarch/kvm/lsvz.c
index fcac2b760013..3cc005f3ea62 100644
--- a/arch/loongarch/kvm/lsvz.c
+++ b/arch/loongarch/kvm/lsvz.c
@@ -2116,7 +2116,7 @@ static int kvm_vz_set_one_reg(struct kvm_vcpu *vcpu,
        case KVM_CSR_PRCFG3:
                break;
        case KVM_CSR_KSCRATCH0 ... KVM_CSR_KSCRATCH7:
-               idx = reg->id - KVM_CSR_KSCRATCH0;
+               idx = (reg->id - KVM_CSR_KSCRATCH0) / 8;
                switch (idx) {
                case 0:
                        write_gcsr_kscratch0(v);
```

## VDSO 的特殊注意
因为 vdso 的系统调用是不会触发 hypercall 的，而是使用 timerid 作为 index 来索引
vdso 中的一个数组，必须保证 host 中 timerid 和 guest 的 timerid 是同步才能保证那些
guest 的调用 vdso 系统调用是正确的。

暂时的解决办法是在进入 guest 之前同步一下 timerid
```diff
+++ b/arch/loongarch/kvm/lsvz.c
@@ -2668,6 +2671,8 @@ static int kvm_vz_vcpu_run(struct kvm_run *run, struct kvm_vcpu *vcpu)
        kvm_vz_check_requests(vcpu, cpu);
        kvm_vz_vcpu_load_tlb(vcpu, cpu);

+       write_gcsr_timerid(smp_processor_id());
        r = vcpu->arch.vcpu_run(run, vcpu);

        return r;
```
example/vdso.c 是一个简单的测试。
