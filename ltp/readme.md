# Test with ltp
check out to commit hash : 5b0b0184b35a09857f4d4ff05dc3c3c648502cf9 

clone the ltp to `/home/loongson/ltp`, if the dir changed, change the script.
## Step one : setup the baseline
1. follow readme's instructions, compile a ltp and install it
    - maybe add-stdint.patch is needed, otherwise `ptrace0*.c` would complaint

The patch is fairly simple where we only take care of how testcases/kernel/syscalls works, so compile other modules firstly.
## Step two : modify the benchmark
1. apply `0001-dune.patch` to the ltp
2. execute `add_dune_enter.sh`
3. execute `generate_runtest.sh`
4. cd testcases/kernel/syscalls
    - make clean
    - make -j10
    - make -j10 : this is correct because makefile dependency problem, will be fixed later.
    - sudo make install

5. sudo chmod 666 /dev/kvm
  - some testcases change user to nobody, nobody group has no permission to open kvm.

## Step three : run the benchmark
```sh
sudo cp /home/loongson/ltp/runtest/syscalls-dune /opt/ltp/runtest/syscalls_dune
sudo /opt/ltp/runltp -f syscalls_dune | tee ~/ltp/log.txt
```
## Step four : compare the result
1. TFAIL
2. TBROK
3. Fatal
