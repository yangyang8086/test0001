# =============================================================================
# Generic C++ Makefile
# =============================================================================

# 1. 基本配置 (Basic Configuration)
# -----------------------------------------------------------------------------

# 编译器 (Compiler)
CXX = g++

# 编译选项 (Compiler Flags)
# -std=c++17: 使用 C++17 标准
# -Wall:      开启所有常用警告
# -Wextra:    开启额外的警告
# -g:         生成调试信息 (用于 gdb)
# -O2:        优化级别 (可改为 -O0, -O1, -O3, -Os)
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O2

# 链接器选项 (Linker Flags)
# 例如: -L/usr/local/lib -lmy_library
LDFLAGS =

# 目标可执行文件名 (Target Executable Name)
TARGET = my_app

# 2. 目录与文件 (Directories and Files)
# -----------------------------------------------------------------------------

# 源文件目录 (Source Directory)
SRCDIR = src

# 构建/对象文件目录 (Build/Object Directory)
OBJDIR = obj

# 自动查找所有 .cpp 源文件 (Automatically find all .cpp source files)
SOURCES = $(wildcard $(SRCDIR)/*.cpp)

# 根据源文件生成对象文件名 (Generate object file names from source files)
# e.g., src/main.cpp -> obj/main.o
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

# 生成依赖文件名 (for header dependencies)
# e.g., obj/main.o -> obj/main.d
DEPS = $(OBJECTS:.o=.d)


# 3. Makefile 规则 (Makefile Rules)
# -----------------------------------------------------------------------------

# 默认目标 (Default Target)
# .PHONY 告诉 make 'all' 不是一个文件名
.PHONY: all
all: $(TARGET)

# 链接规则: 将所有对象文件链接成最终的可执行文件
# (Linking rule: link all object files into the final executable)
$(TARGET): $(OBJECTS)
	@echo "LD   $@"
	$(CXX) $(LDFLAGS) $^ -o $@

# 编译规则: 将每个 .cpp 文件编译成 .o 文件
# (Compilation rule: compile each .cpp file into a .o file)
# -MMD -MP: 生成 .d 依赖文件，用于追踪头文件的变化
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D) # 如果构建目录不存在，则创建它 (Create build directory if it doesn't exist)
	@echo "CXX  $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MP

# 清理规则: 删除所有生成的文件
# (Clean rule: remove all generated files)
.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJDIR) $(TARGET)

# 包含生成的依赖文件
# (Include generated dependency files)
# 前面的 '-' 符号表示如果依赖文件不存在，不要报错
-include $(DEPS)


# 4. 帮助 (Help)
# -----------------------------------------------------------------------------
.PHONY: help
help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all       (default) Compile and link the project."
	@echo "  clean     Remove all generated files."
	@echo "  help      Show this help message."
	@echo ""
	@echo "Configuration Variables:"
	@echo "  CXX=$(CXX)"
	@echo "  CXXFLAGS=$(CXXFLAGS)"
	@echo "  LDFLAGS=$(LDFLAGS)"
	@echo "  TARGET=$(TARGET)"
	@echo "  SRCDIR=$(SRCDIR)"
	@echo "  OBJDIR=$(OBJDIR)"
