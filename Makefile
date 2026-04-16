# 编译器路径（构建目标）
COMPILER_BUILD = build/compiler
# 编译器源码目录（假设你的编译器源码在 src/ 或类似目录）
COMPILER_SRC_DIR = .

# 源文件目录
SRC_DIR = debug
# 输出目录
OUT_DIR = output

# 获取所有 .cpp 文件
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
# 生成对应的 .koopa 文件名
TARGETS = $(patsubst $(SRC_DIR)/%.cpp, $(OUT_DIR)/%.koopa, $(SOURCES))

# 默认目标：先构建编译器，再编译所有测试文件
all: $(COMPILER_BUILD) $(TARGETS)

# 构建编译器的规则
$(COMPILER_BUILD):
	@echo "Building compiler..."
	@cd build && $(MAKE)
	@echo "Compiler built successfully"

# 创建输出目录
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# 编译规则：%.koopa 依赖于对应的 %.cpp 和编译器
$(OUT_DIR)/%.koopa: $(SRC_DIR)/%.cpp $(COMPILER_BUILD) | $(OUT_DIR)
	$(COMPILER_BUILD) -koopa $< -o $@

# 清理所有（包括编译器和输出文件）
clean:
	rm -rf $(OUT_DIR)
	@echo "Cleaning build..."
	@cd build && $(MAKE) clean 2>/dev/null || true

# 只清理输出文件，不清理编译器
clean-output:
	rm -rf $(OUT_DIR)

# 查看某个文件的编译结果（比如 hello.cpp）
show-hello: $(COMPILER_BUILD) $(OUT_DIR)/hello.koopa
	cat $(OUT_DIR)/hello.koopa

# 一行命令编译并查看
quick: $(COMPILER_BUILD) $(OUT_DIR)/hello.koopa
	cat $(OUT_DIR)/hello.koopa

# 测试所有文件
test: all
	@echo "=== Generated files ==="
	@ls -la $(OUT_DIR)/

.PHONY: all clean clean-output show-hello quick test