# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
LDFLAGS =

# Target executable
TARGET = imcsh

# Source files
SRCS = imcsh.c

# Object files (automatically derived from source files)
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets (not actual files)
.PHONY: all clean