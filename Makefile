CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -lrt -pthread
SOURCES = $(wildcard *.c)
NAME = proj2

compile:
	$(CC) $(CFLAGS) $(SOURCES) -o $(NAME)

debug:
	$(CC) -g $(CFLAGS) $(SOURCES) -o $(NAME)

run: compile
	./$(NAME)

d_run: debug
	gdb $(NAME)
