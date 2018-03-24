#!/bin/bash
make clean;make
./sim test1 test1_elf.txt -a > test1_sim.txt 
./sim test2 test2_elf.txt -a > test2_sim.txt 
./sim test5 test5_elf.txt -a > test5_sim.txt 
./sim test8 test8_elf.txt -a > test8_sim.txt 
./sim test10 test10_elf.txt -a > test10_sim.txt 
