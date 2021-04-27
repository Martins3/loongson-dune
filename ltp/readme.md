# Test with ltp

ltp commit hash : 5b0b0184b35a09857f4d4ff05dc3c3c648502cf9 

## Step one
1. follow readme's instructions, compile a ltp and install it
    - maybe add-stdint.patch is needed, otherwise `ptrace0*.c` would complaint

The patch is fairly simple where we only take care of how testcases/kernel/syscalls works, so
compile other modules firstly.

## Step two
clone the ltp to /home/loongson/ltp, if the dir changed, change the script

- [ ] Maybe we should define variable env_pre.mk

1. format dune.patch to the ltp
2. execute `add_dune_enter.sh`
3. execute `generate_runtest.sh`
4. cd testcases/kernel/syscalls
    - make clean
    - make -j10
    - make -j10 : this is correct because makefile dependency problem, will be fixed later.
    - sudo make install

## Step three
sudo cp /home/loongson/ltp/runtest/syscalls-dune /opt/ltp/runtest/syscalls_dune
sudo /opt/ltp/runltp -f syscalls_dune | tee log.txt
