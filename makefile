CC := gcc

DEBUG_FLAGS 	:= -g -Wall -Wextra -Wpedantic #-fsanitize=address
RELEASE_FLAGS 	:= 
LD_FLAGS 		:= $$(pkg-config --libs glfw3) $$(pkg-config --libs gl) $$(pkg-config --libs glew) $$(pkg-config --libs cglm) -lm

BUILD_DIR 	:= build
SRC_DIR		:= src

SRC    := $(wildcard $(SRC_DIR)/*.c)
TARGET := $(BUILD_DIR)/marching_cubes

.PHONY: build
build:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(DEBUG_FLAGS) $(SRC) -o $(TARGET) $(LD_FLAGS) 

run: build
	@$(TARGET) $(ARGS)

debug: build
	@gdb $(TARGET)