DIR=.
BIN_DIR=$(DIR)/bin
LIB_DIR=$(DIR)/lib
SRC_DIR=$(DIR)/src
TEST_DIR=$(DIR)/test
INCLUDE_DIR=$(DIR)/
OBJ_DIR=$(DIR)/obj
DEPS_DIR=$(DIR)/deps
PROGRAM=$(BIN_DIR)/beacond
TEST_DIR=./test

EXTENSION=cc
OBJS=$(patsubst $(SRC_DIR)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.$(EXTENSION)))
TEST_OBJS=$(patsubst $(TEST_DIR)/%.$(EXTENSION), $(TEST_DIR)/%.o,$(wildcard $(TEST_DIR)/*.$(EXTENSION)))
DEPS=$(patsubst $(OBJ_DIR)/%.o, $(DEPS_DIR)/%.d, $(OBJS))
SRC_OBJS=./obj/buffer.o  ./obj/net.o  ./obj/redis_command.o  ./obj/server.o  ./obj/logger.o  ./obj/redis_session.o  ./obj/redis_item.o  ./obj/epoll.o ./obj/util.o  ./obj/bitmap.o  ./obj/engine.o  ./obj/redis_parser.o
INCLUDE= -I./src

CC=g++
CFLAGS=-Wall -Werror -g $(MYCFLAGS)
LDFLAGS= -L -lpthread

MYCFLAGS = 

all: $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS) $(LDFLAGS) 

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.$(EXTENSION) 
	$(CC) $< -o $@ -c $(CFLAGS) $(INCLUDE) 

test:$(TEST_OBJS) $(OBJS)
	$(CC) $(TEST_OBJS) -o test/all_test -lgtest -lpthread $(SRC_OBJS)

$(TEST_DIR)/%.o:$(TEST_DIR)/%.$(EXTENSION)
	$(CC) $< -o $@ -c $(CFLAGS) $(INCLUDE)

rebuild:
	make clean
	make

clean:
	rm -rf  $(OBJ_DIR)/* $(BIN_DIR)/*
