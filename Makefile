FLAGS=-lrt -ggdb
CC=gcc

main: main_poll.c buffer.c process_child.c memfile.c
	$(CC) main_poll.c $(FLAGS) -o main

