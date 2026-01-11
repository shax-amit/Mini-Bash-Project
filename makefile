# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Target executable name
TARGET = mini_bash

# Default target: compile the executable
all: $(TARGET)

$(TARGET): mini_bash.c
	$(CC) $(CFLAGS) -o $(TARGET) mini_bash.c

# Clean target: remove the executable
clean:
	rm -f $(TARGET)