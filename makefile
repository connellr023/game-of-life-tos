# Directories
SRC_DIR_NAME = src
BUILD_DIR_NAME = build

SRC_DIR = ./$(SRC_DIR_NAME)
BUILD_DIR = ./$(BUILD_DIR_NAME)
INCLUDE_DIR = ./include
LIB_RPI3_DRIVERS_DIR = ./rpi3-drivers
TRANSIENT_OS_DIR = ./transient-os

# Find all .c, .cpp, and .s files recursively in the source directory
SRCS = $(shell find $(SRC_DIR) -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.s' \))
OBJS = $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(SRCS:.c=.o))
OBJS := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(OBJS:.cpp=.o))
OBJS := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(OBJS:.s=.o))

# Static libraries
LIBS = $(LIB_RPI3_DRIVERS_DIR)/build/librpi3-drivers.a $(TRANSIENT_OS_DIR)/build/libtransient-os.a

# Compiler and linker flags
CFLAGS = -O1 -nostdlib -fno-exceptions -fno-rtti -Wall -Wextra -I$(INCLUDE_DIR) -I$(LIB_RPI3_DRIVERS_DIR)/include -I$(TRANSIENT_OS_DIR)/include

# Output executable
TARGET = $(BUILD_DIR)/kernel8.img

# Default target
all: $(LIBS) $(TARGET)

# Build the ELF and IMG files
$(TARGET): $(OBJS) $(LIBS)
	ld.lld -m aarch64elf $(OBJS) \
		$(LIB_RPI3_DRIVERS_DIR)/build/librpi3-drivers.a \
		$(TRANSIENT_OS_DIR)/build/libtransient-os.a \
		-T $(TRANSIENT_OS_DIR)/link.ld -o $(BUILD_DIR)/kernel8.elf
	llvm-objcopy -O binary $(BUILD_DIR)/kernel8.elf $(TARGET)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $< to $@"
	clang++ --target=aarch64-elf $(CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Build submodule libraries
$(LIB_RPI3_DRIVERS_DIR)/build/librpi3-drivers.a:
	$(MAKE) -C $(LIB_RPI3_DRIVERS_DIR)

$(TRANSIENT_OS_DIR)/build/libtransient-os.a:
	$(MAKE) -C $(TRANSIENT_OS_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR_NAME)
	$(MAKE) -C $(LIB_RPI3_DRIVERS_DIR) clean
	$(MAKE) -C $(TRANSIENT_OS_DIR) clean

.PHONY: all clean