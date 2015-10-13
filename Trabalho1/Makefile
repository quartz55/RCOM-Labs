CC = clang
CFLAGS = -Wall -g -O3
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

write : writenoncanonical.o
	$(CC) $(LIBS) $< -o $@

read : noncanonical.o
	$(CC) $(LIBS) $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $<

.PHONY : clean

clean:
	rm -rf $(OBJ) read write
