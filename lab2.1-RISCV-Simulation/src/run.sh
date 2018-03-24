#!/bin/bash
make clean;make
./main add add_elf.txt -a > add_sim.txt 
./main mul-div mul-div_elf.txt -a > mul-div_sim.txt
./main simple-fuction simple-fuction_elf.txt -a > simple-fuction_sim.txt
./main n! factorial_elf.txt -a > factorial_sim.txt
./main qsort qsort_elf.txt -a > qsort_sim.txt
