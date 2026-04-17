#include "riscv.h"
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
        default:
            // 其他指令类型暂不支持，可以忽略或报错
            // 对于简单程序，我们只关心 return 和 integer
            break;
    }
}

void RISCVGenerator::visit_return(const koopa_raw_return_t &ret) {
    if (ret.value) {
        // 有返回值，需要先处理返回值
        const auto &value_kind = ret.value->kind;
        if (value_kind.tag == KOOPA_RVT_INTEGER) {
            // 直接返回整数
            int32_t int_val = value_kind.data.integer.value;
            load_immediate(0, int_val);
        } else {
            // 其他类型的返回值（后续扩展）
            // 例如从寄存器加载
        }
    } else {
        // void 函数，返回 0
        load_immediate(0, 0);
    }
    os << "  ret\n";
}

void RISCVGenerator::visit_integer(const koopa_raw_integer_t &integer) {
    // 整数常量通常在 return 指令中被处理
    // 这里暂时不需要单独处理
    (void)integer;
}