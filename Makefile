# 根目录 Makefile
.PHONY: all clean

# autotest 会传入 BUILD_DIR 变量, 默认为当前目录
BUILD_DIR ?= .

all:
	@mkdir -p build
	@cd build && cmake .. && $(MAKE)
	@# 将编译产物拷贝到 autotest 指定的目录, 如果没指定则拷贝到根目录
	@cp build/compiler $(BUILD_DIR)/compiler

clean:
	@rm -rf build compiler