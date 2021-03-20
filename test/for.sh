#!/bin/bash
dir=/home/loongson/dune/test/pthread

for((i=0; i < 10; i++)); do
  echo $i
  cmd= $dir/dune-fcntl34
  $cmd
  if [[ $? -ne 0 ]]; then
    exit 1
  fi
done

