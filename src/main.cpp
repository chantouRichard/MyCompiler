#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include "ast.h"
#include "riscv.h"

// 新增：Koopa 头文件
#include <koopa.h>

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
extern FILE *yyin;
extern int yyparse(unique_ptr<CompUnit> &ast);

// 函数声明
bool koopa_to_riscv(const string &koopa_ir, const string &output_file);
bool generate_output(const string &mode, CompUnit *ast, const string &output);

int main(int argc, const char *argv[]) {
  // 解析命令行参数
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  // 打开输入文件
  yyin = fopen(input, "r");
  if (!yyin) {
    cerr << "Error: Cannot open input file: " << input << endl;
    return 1;
  }

  // 调用 parser 函数，得到 AST
  unique_ptr<CompUnit> ast;
  auto ret = yyparse(ast);
  
  // 关闭输入文件
  fclose(yyin);
  
  // 检查解析是否成功
  if (ret != 0 || !ast) {
    cerr << "Error: Parsing failed" << endl;
    return 1;
  }

  // 根据 mode 决定输出格式
  // 目前支持 koopa 格式（打印 AST 结构）
  
  // 创建并写入输出文件
  ofstream out_file(output);
  if (!out_file) {
    cerr << "Error: Cannot create output file: " << output << endl;
    return 1;
  }
  
  // 生成输出
  if (!generate_output(mode, ast.get(), output)) {
    return 1;
  }
  
  return 0;
}

/**
 * 将文本 Koopa IR 转换为 RISC-V 汇编
 * @param koopa_ir 文本形式的 Koopa IR
 * @param output_file 输出文件路径
 * @return 是否成功
 */
bool koopa_to_riscv(const string &koopa_ir, const string &output_file) {
    // 1. 解析字符串，得到 Koopa IR 程序
    koopa_program_t program;
    // printf("koopa_ir.c_str: %s\n", koopa_ir.c_str());
    koopa_error_code_t ret = koopa_parse_from_string(koopa_ir.c_str(), &program);
    
    if (ret != KOOPA_EC_SUCCESS) {
        cerr << "Error: Failed to parse Koopa IR" << endl;
        return false;
    }
    
    // 2. 创建 raw program builder
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    
    // 3. 将 Koopa IR 程序转换为 raw program
    koopa_raw_program_t raw_program = koopa_build_raw_program(builder, program);
    
    // 4. 生成 RISC-V 汇编
    ofstream out_file(output_file);
    if (!out_file) {
        cerr << "Error: Cannot create output file: " << output_file << endl;
        koopa_delete_program(program);
        koopa_delete_raw_program_builder(builder);
        return false;
    }
    
    RISCVGenerator generator(out_file);
    generator.generate(raw_program);
    out_file.close();
    
    // 5. 释放资源
    koopa_delete_program(program);
    koopa_delete_raw_program_builder(builder);
    
    return true;
}

/**
 * 根据 mode 生成对应的输出
 * @param mode 运行模式（-koopa, -riscv 等）
 * @param ast AST 根节点
 * @param output 输出文件路径
 * @return 是否成功
 */
bool generate_output(const string &mode, CompUnit *ast, const string &output) {
    // 使用字符串流收集输出，而不是直接写入文件
    ostringstream oss;
    
    // 根据模式选择 Visitor
    if (mode == "-koopa") {
        // 生成文本形式的 Koopa IR
        KoopaVisitor koopa_visitor(oss);
        ast->accept(&koopa_visitor);
        
        // 获取生成的 IR 文本
        string koopa_ir = oss.str();
        
        // 写入文件
        ofstream out_file(output);
        if (!out_file) {
            cerr << "Error: Cannot create output file: " << output << endl;
            return false;
        }
        out_file << koopa_ir;
        out_file.close();
        
    } else if (mode == "-ast") {
        // 调试模式：输出 AST 结构
        PrintVisitor printer(oss);
        ast->accept(&printer);
        
        ofstream out_file(output);
        if (!out_file) {
            cerr << "Error: Cannot create output file: " << output << endl;
            return false;
        }
        out_file << oss.str();
        out_file.close();
        
    } else if (mode == "-riscv") {
        // 1. 先生成 Koopa IR 到字符串流
        KoopaVisitor koopa_visitor(oss);
        ast->accept(&koopa_visitor);
        string koopa_ir = oss.str();
        
        // 2. 将 Koopa IR 转换为 RISC-V 汇编
        if (!koopa_to_riscv(koopa_ir, output)) {
            return false;
        }
    } else {
        cerr << "Error: Unsupported mode: " << mode << endl;
        return false;
    }
    
    return true;
}