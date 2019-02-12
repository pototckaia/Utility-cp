compile: cp.o main.o  
	gcc cp.o main.o -o cp
	rm *.o 

cp.o: cp.c cp.h
	gcc -c cp.c 

main.o: main.c cp.h
	gcc -c main.c

clear:
	rm *.o


