main:	Read_Elf.o Simulation.o
	g++ Read_Elf.o Simulation.o -o main
Simulation.o:
	g++ -c multicycle_Simulation.cpp -o Simulation.o 2>err
Read_Elf.o:
	g++ -c Read_Elf.cpp -o Read_Elf.o 2>err 
clean:
	rm *.o 
