#ifndef RISCV_H
#define RISCV_H

#include <iostream>
#include <string>
#include "koopa.h"
#include <map>

// RISC-V 汇编生成器
class RISCVGenerator {
private:
    std::ostream &os;
    int label_counter;  // 用于生成唯一标签
    
    // 辅助函数：生成唯一标签名
    std::string new_label();
    
    // 辅助函数：打印缩进
    void print_indent(int indent = 1);
    
    // 辅助函数：加载立即数到寄存器
    void load_immediate(int reg, int32_t value);

    // 去掉 Koopa 名称前缀（@ 或 %）
    std::string strip_prefix(const char* name);

    // 记录指令对应的结果寄存器
    std::map<koopa_raw_value_t, std::string> value_to_reg;
    int next_reg_idx = 0;
    std::string temp_regs[7] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6"};

    // 分配下一个可用寄存器 (简单演示，不考虑耗尽情况)
    std::string allocate_reg() { return temp_regs[next_reg_idx++]; }
    
public:
    explicit RISCVGenerator(std::ostream &out);
    
    // 生成 RISC-V 汇编的入口函数
    void generate(const koopa_raw_program_t &program);
    
    // 各个组件的访问函数
    void visit_program(const koopa_raw_program_t &program);
    void visit_function(const koopa_raw_function_t &func);
    void visit_basic_block(const koopa_raw_basic_block_t &bb);
    void visit_value(const koopa_raw_value_t &value);
    
    // 具体指令的访问函数
    void visit_return(const koopa_raw_return_t &ret);
    void visit_integer(const koopa_raw_integer_t &integer);
    void visit_binary(const koopa_raw_value_t &value);
    void load_value_to_reg(const koopa_raw_value_t &value, const std::string &reg);
    // 后续可以添加更多指令类型
    std::string get_value_src(const koopa_raw_value_t &value);
};

#endif // RISCV_H