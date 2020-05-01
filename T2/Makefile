all: utilidades.c main.c
	gcc -o main *.c
	./main
compile:
	gcc -o main *.c
run:
	./main
memory_test:
	gcc -g -o main *.c
	valgrind --leak-check=full ./main
