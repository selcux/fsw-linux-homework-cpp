# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I include

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
SRCS = $(SRC_DIR)/client1.cpp

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Executables
TARGET = $(BUILD_DIR)/client1

# Default target
all: prepare $(TARGET)

# Prepare build directory
prepare:
	@mkdir -p $(BUILD_DIR)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link the main executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

# Run the main program
run: all
	./$(TARGET)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all prepare run clean