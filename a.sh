#!/bin/bash
list=(10 12 14 16 20)
echo "" > aaa;
for i in ${list[@]}
do
  printf "${i} p>\n" >>aaa;
  ./rng 100000000 input.dat;
  mpirun -n "${i}" --oversubscribe ./scatter input.dat 1>/dev/null 2>>aaa;
  printf "\n" >>aaa;
done
