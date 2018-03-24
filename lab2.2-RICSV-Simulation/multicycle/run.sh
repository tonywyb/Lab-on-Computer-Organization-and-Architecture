#!/bin/bash
make clean;make
./main test1 test1_elf.txt -a > test1_sim.txt 
./main test2 test2_elf.txt -a > test2_sim.txt 
./main test3 test3_elf.txt -a > test3_sim.txt 
./main test4 test4_elf.txt -a > test4_sim.txt 
./main test5 test5_elf.txt -a > test5_sim.txt 
./main test6 test6_elf.txt -a > test6_sim.txt 
./main test7 test7_elf.txt -a > test7_sim.txt 
./main test8 test8_elf.txt -a > test8_sim.txt 
./main test9 test9_elf.txt -a > test9_sim.txt 
./main test10 test10_elf.txt -a > test10_sim.txt 
