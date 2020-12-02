CC		=	g++
SRC_DIR	=	src
LIB_DIR	=	include
BIN_DIR	=	bin
TARGET	=	main

CFLAGS	=	-I$(LIB_DIR) -lssl -lcrypto
SRC		=	$(wildcard $(SRC_DIR)/*.cpp)
OBJ		=	$(SRC:%.cpp=%.o)

$(TARGET): $(OBJ) $(TARGET).o
	$(CC) -o $@ $@.o $(OBJ) $(CFLAGS)

$(TARGET).o: $(TARGET).cpp
	$(CC) -c -o $@ $< $(CFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)
