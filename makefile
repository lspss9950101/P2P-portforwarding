CC		=	gcc
SRC_DIR	=	src
LIB_DIR	=	include
BIN_DIR	=	bin
TARGET	=	main

CFLAGS	=	-I$(LIB_DIR)
SRC		=	$(wildcard $(SRC_DIR)/*.c)
OBJ		=	$(SRC:%.c=%.o)

$(TARGET): $(OBJ)
	$(CC) -o main $@.c $(CFLAGS) $^

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)
