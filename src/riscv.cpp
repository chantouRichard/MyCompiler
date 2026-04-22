#include "riscv.h"
#include <iostream>
#include <cassert>

RISCVGenerator::RISCVGenerator(std::ostream &out) 
    : os(out), label_counter(0) {}

// 去掉 Koopa 名称前缀（@ 或 %）
std::string RISCVGenerator::strip_prefix(const char* name) {
    if (!name) return "";
    std::string result = name;
    if (!result.empty() && (result[0] == '@' || result[0] == '%')) {
        result = result.substr(1);
    }
    return result;
}

std::string RISCVGenerator::new_label() {
    return ".L" + std::to_string(label_counter++);
}

void RISCVGenerator::print_indent(int indent) {
    for (int i = 0; i < indent; ++i) {
        os << "  ";
    }
}

void RISCVGenerator::load_immediate(int reg, int32_t value) {
    os << "  li a" << reg << ", " << value << "\n";
}

void RISCVGenerator::generate(const koopa_raw_program_t &program) {
    // 输出汇编头部
    os << "  .text\n";
    os << "  .global main\n";  // 假设有 main 函数
    
    // 遍历所有函数
    visit_program(program);
}

void RISCVGenerator::visit_program(const koopa_raw_program_t &program) {
    // 遍历函数列表
    for (size_t i = 0; i < program.funcs.len; ++i) {
        assert(program.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func = 
            reinterpret_cast<koopa_raw_function_t>(program.funcs.buffer[i]);
        visit_function(func);
    }
}

void RISCVGenerator::visit_function(const koopa_raw_function_t &func) {
    // 输出函数标签
    os << strip_prefix(func->name) << ":\n";
    // printf("function name: %s\n", func->name);
    
    // 遍历基本块
    for (size_t i = 0; i < func->bbs.len; ++i) {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb = 
            reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
        visit_basic_block(bb);
    }
}

void RISCVGenerator::visit_basic_block(const koopa_raw_basic_block_t &bb) {
    // 检查是否需要输出标签
    // 条件：基本块有名称，且名称不是默认的 "entry"，且该基本块被其他指令引用
    bool need_label = false;
    if (bb->name) {
        std::string label_name = strip_prefix(bb->name);
        // 如果标签名不是 "entry"，或者该基本块有多个前驱，则需要输出标签
        if (label_name != "entry" || bb->used_by.len > 0) {
            need_label = true;
            os << label_name << ":\n";
        }
    }
    
    // 遍历指令
    for (size_t i = 0; i < bb->insts.len; ++i) {
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        koopa_raw_value_t value = 
            reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[i]);
        visit_value(value);
    }
}

void RISCVGenerator::visit_value(const koopa_raw_value_t &value) {
    const auto &kind = value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            visit_return(kind.data.ret);
            break;
        case KOOPA_RVT_INTEGER:
            visit_integer(kind.data.integer);
            break;
        case KOOPA_RVT_BINARY:
            visit_binary(value);
            break;
        default:
            // 其他指令类型暂不支持，可以忽略或报错
            // 对于简单程序，我们只关心 return 和 integer
            // std::cout<< "kind.tag: "<< kind.tag << std::endl;
            break;
    }
}

void RISCVGenerator::visit_return(const koopa_raw_return_t &ret) {
    if (ret.value) {
        // 统一调用辅助函数，将返回值加载到 a0
        load_value_to_reg(ret.value, "a0");
    } else {
        os << "  li a0, 0\n";
    }
    os << "  ret\n";
}

void RISCVGenerator::visit_integer(const koopa_raw_integer_t &integer) {
    // 整数常量通常在 return 指令中被处理
    // 这里暂时不需要单独处理
    (void)integer;
}

void RISCVGenerator::visit_binary(const koopa_raw_value_t &value) {
const auto &binary = value->kind.data.binary;
    
    // 1. 为当前指令的结果分配寄存器
    std::string res_reg = allocate_reg();
    value_to_reg[value] = res_reg;

    // 2. 处理左操作数 (lhs)
    // 如果左操作数是 0，我们不需要 li，直接用 x0
    // 如果是普通数字，我们直接把数字 li 到 res_reg 中
    std::string lhs_reg;
    if (binary.lhs->kind.tag == KOOPA_RVT_INTEGER && binary.lhs->kind.data.integer.value != 0) {
        os << "  li    " << res_reg << ", " << binary.lhs->kind.data.integer.value << "\n";
        lhs_reg = res_reg;
    } else {
        lhs_reg = get_value_src(binary.lhs);
    }

    // 3. 获取右操作数 (rhs) 寄存器
    std::string rhs_reg = get_value_src(binary.rhs);
    switch (binary.op) {
        case KOOPA_RBO_ADD:
            os << "  add a0, t0, t1\n";
            break;
        case KOOPA_RBO_SUB:
            os << "  sub " << res_reg << ", "<< lhs_reg <<", "<< rhs_reg <<"\n";
            break;
        case KOOPA_RBO_MUL:
            os << "  mul a0, t0, t1\n";
            break;
        case KOOPA_RBO_DIV:
            os << "  div a0, t0, t1\n";
            break;
        case KOOPA_RBO_MOD:
            os << "  rem a0, t0, t1\n";
            break;
        case KOOPA_RBO_EQ:
            os << "  sub " << res_reg << ", "<< lhs_reg <<", "<< rhs_reg <<"\n";
            os << "  seqz " << res_reg << ", " << res_reg << "\n";
            break;
        case KOOPA_RBO_NOT_EQ:
            os << "  sub " << res_reg << ", "<< lhs_reg <<", "<< rhs_reg <<"\n";
            os << "  snez " << res_reg << ", " << res_reg << "\n";
            break;
        case KOOPA_RBO_LT:
            os << "  slt a0, t0, t1\n";
            break;
        case KOOPA_RBO_GT:
            os << "  slt a0, t1, t0\n";
            break;
        // ... 其他操作符类似实现
        default:
            break;
    }
}

void RISCVGenerator::load_value_to_reg(const koopa_raw_value_t &value, const std::string &reg) {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        int32_t val = value->kind.data.integer.value;
        if (val == 0) {
            // 如果是 0，直接使用 x0，无需 li
            if (reg != "x0") os << "  mv " << reg << ", x0\n";
        } else {
            os << "  li " << reg << ", " << val << "\n";
        }
    } else {
        // 如果不是常量，它一定已经在之前的指令中计算过并存入寄存器了
        std::string src_reg = value_to_reg[value];
        if (src_reg != reg) os << "  mv " << reg << ", " << src_reg << "\n";
    }
}

std::string RISCVGenerator::get_value_src(const koopa_raw_value_t &value) {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        if (value->kind.data.integer.value == 0) return "x0";
        // 非 0 常量仍需加载到临时寄存器，这里可以用一个不被映射占用的寄存器如 t6
        std::string temp = allocate_reg();
        os << "  li " << temp << ", " << value->kind.data.integer.value << "\n";
        return temp;
    }
    return value_to_reg[value]; // 返回之前计算存入的 t0, t1 等
}