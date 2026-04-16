#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include "ast.h"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
extern FILE *yyin;
extern int yyparse(unique_ptr<CompUnit> &ast);

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
  
  // 将 AST 以文本形式写入文件
  ast->print(out_file);
  out_file.close();
  
  // 可选：同时打印到屏幕
  // ast->print(cout);
  
  return 0;
}