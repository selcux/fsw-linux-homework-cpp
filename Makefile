# Define the build directory
BUILD_DIR = ./build

# Define the target executable name
TARGET = fsw_linux_homework_cpp

# Default target
all: build

# Build target
build:
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

# Run target
run: $(BUILD_DIR)/$(TARGET)
	@./$(BUILD_DIR)/$(TARGET)

# Clean target
clean:
	@rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all build clean run