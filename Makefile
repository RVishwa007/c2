# Compiler and Linker configurations
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -MMD -MP
LDFLAGS = -lncursesw -Wl,-subsystem,console

# Directory structure
SRC_DIR = src
OBJ_DIR = obj

# Source and Object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Executable target name
TARGET = nexus.exe

# Default target
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compiling source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Include generated dependency files
-include $(OBJS:.o=.d)

# Run the compiled application
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean run
