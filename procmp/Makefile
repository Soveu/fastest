CC=g++
WARNINGS=-Wall -Wextra -Wshadow
FLAGS=-O3 -march=native $(WARNINGS)
FILES=buffer.cpp pchild.cpp duplicate_fd_content.cpp
SH=/bin/bash
TEST=./procmp $(SH) $(SH)

procmp: main.cpp $(FILES)
	g++ main.cpp $(FLAGS) -o procmp

clean:
	rm procmp

test:
	time echo -e 'echo \x24RANDOM' | $(TEST)
	time $(TEST) < case

