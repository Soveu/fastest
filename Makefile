FLAGS=-lrt -ggdb
CC=gcc

main: main.c buffer.c process_child.c memfile.c
	$(CC) main.c $(FLAGS) -o main

