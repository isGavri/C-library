CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDFLAGS = -lncurses

# Directories
SRC_DIR = src
OBJ_DIR = bin
INC_DIR = include
TEST_DIR = test

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)

# Target executable
TARGET = $(OBJ_DIR)/monkeytype
TEST_TARGET = $(OBJ_DIR)/test_runner

# Default target
all: $(TARGET)

# Main executable
$(TARGET): $(OBJECTS) | $(OBJ_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Test object files
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Test target (exclude main.o from tests)
test: $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS)) $(TEST_OBJECTS) | $(OBJ_DIR)
	$(CC) $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS)) $(TEST_OBJECTS) -o $(TEST_TARGET) $(LDFLAGS)
	./$(TEST_TARGET)

# Create bin directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean build files
clean:
	rm -rf $(OBJ_DIR)/*

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install libncurses5-dev libncurses5

# Generate compile_commands.json for LSP
compile_commands:
	bear -- make clean all

# Run the program
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

.PHONY: all test clean install-deps compile_commands run debug
