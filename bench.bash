#!/bin/bash


g++ -fopenmp -o omp omp.cpp

touch omp.txt
#

# build
# g++ -fopenmp -o default default.cpp
# default
# for elements in 100000 1000000 2000000; do
#   hyperfine --runs 10 --warmup 1 "./default $elements" --shell=none --export-json "./results/default/default-elements-$elements.json"
# done

# openmp
for threads in   8  7  6  5  4  3  2  1; do
  echo " " >> omp.txt
  node in_generator.js 10000 2000 2000 400 | ./omp $threads >> omp.txt
done

