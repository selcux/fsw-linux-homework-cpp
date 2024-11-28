# Define the build directory
BUILD_DIR = ./build

# Define the target executable names
TARGET1 = client1
TARGET2 = client2

# Default target
all: build

# Build target
build:
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

run-client1: $(BUILD_DIR)/$(TARGET1)
	@./$(BUILD_DIR)/$(TARGET1)

run-client2: $(BUILD_DIR)/$(TARGET2)
	@./$(BUILD_DIR)/$(TARGET2)

# Clean target
clean:
	@rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all build clean run-client1 run-client2