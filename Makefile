CC := clang
CFLAGS := -std=c17 -Wall -Wextra -Iinclude/
RELEASE_CFLAGS := -Werror -O3
TEST_CFLAGS := -O1 -g -fsanitize=address
LDFLAGS := -lSDL2 -lSDL2_ttf

SRC_DIR := src
BUILD_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.c)

ALL_SRCS = $(SRCS)

RELEASE_OUTPUT := $(BUILD_DIR)/TileEditor
TEST_OUTPUT := $(BUILD_DIR)/test

all: release

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

release: $(BUILD_DIR) $(RELEASE_OUTPUT)

$(RELEASE_OUTPUT): $(ALL_SRCS)
	@echo "Compiling release build..."
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) $^ $(LDFLAGS) -o $@
	@echo "Build successful: $(RELEASE_OUTPUT)"

.PHONY: test
test: $(BUILD_DIR) $(TEST_OUTPUT)

$(TEST_OUTPUT): $(ALL_SRCS)
	@echo "Compiling test build..."
	$(CC) $(CFLAGS) $(TEST_CFLAGS) $^ $(LDFLAGS) -o $@
	@echo "Build successful: $(TEST_OUTPUT)"

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build files."