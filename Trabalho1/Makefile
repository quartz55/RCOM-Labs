CC = clang
CFLAGS = -Wall -g -O3
SRC = writenoncanonical.c
OBJ = $(SRC:.c=.o)
NAME = nserial

$(NAME) : $(OBJ)
	$(CC) $(LIBS) $(OBJ) -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $<

.PHONY : clean

clean:
	rm -rf $(OBJ) $(NAME)
