CC = gcc
LIBS =
CFLAGS = -Wall -g -O3

SRC_DIR = src/
OBJ_DIR = obj/
OUT_DIR = bin/

SRC = $(wildcard $(SRC_DIR)*.c)
OBJ = $(SRC:$(SRC_DIR)%.c=$(OBJ_DIR)%.o)

VPATH = SRC_DIR:OBJ_DIR:OUT_DIR

all: protocol

protocol : $(addprefix $(OBJ_DIR), link.o llframe.o app.o main.o) | bin
	$(CC) $(LIBS) $^ -o $(OUT_DIR)$@

$(OBJ_DIR)%.o : $(SRC_DIR)%.c | obj
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY : clean

clean :
	rm -f $(OBJ_DIR)*.o $(OUT_DIR)protocol

fullclean : clean
	rmdir $(OBJ_DIR) $(OUT_DIR)

info :
	@echo "Source files:" $(SRC)
	@echo "Obj files:" $(OBJ)
	@echo "Bin dir:" $(OUT_DIR)

obj :
	@mkdir -p $(OBJ_DIR)
bin :
	@mkdir -p $(OUT_DIR)
