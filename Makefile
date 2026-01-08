# Compiler settings
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = .

# File lists
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BIN_DIR)/memsim

# Default build target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build successful! Run with ./memsim"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(TARGET) outputs

.PHONY: all clean