DIR=.
BIN_DIR=$(DIR)/bin
LIB_DIR=$(DIR)/lib
SRC_DIR=$(DIR)/src
INCLUDE_DIR=$(DIR)/
OBJ_DIR=$(DIR)/obj
DEPS_DIR=$(DIR)/deps
PROGRAM=$(BIN_DIR)/beacond
TEST_DIR=./test

EXTENSION=cc
OBJS=$(patsubst $(SRC_DIR)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.$(EXTENSION)))
DEPS=$(patsubst $(OBJ_DIR)/%.o, $(DEPS_DIR)/%.d, $(OBJS))

INCLUDE= -I$(INCLUDE_DIR) -I$(LUA_DIR)/src  -I$(LDB_DIR)/src

CC=g++
CFLAGS=-Wall -Werror -g $(MYCFLAGS)
LDFLAGS= -L -lpthread

MYCFLAGS = 

all: $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS) $(LDFLAGS) 

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.$(EXTENSION) 
	$(CC) $< -o $@ -c $(CFLAGS) $(INCLUDE) 

test:all
	cd $(TEST_DIR) && make all

rebuild:
	make clean
	make

clean:
	rm -rf  $(OBJ_DIR)/* $(BIN_DIR)/*
