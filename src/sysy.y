%code requires {
  #include <memory>
  #include <string>
  #include "ast.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<CompUnit> &ast, const char *s);  // 改为 CompUnit

using namespace std;

// 定义 Bison 要调用的 lexer 函数
extern int yylex();

%}

%parse-param { std::unique_ptr<CompUnit> &ast }

%union {
  std::string *str_val;
  int int_val;
  // AST 节点指针
  CompUnit *comp_unit;
  FuncDef *func_def;
  Block *block;
  Stmt *stmt;
  Number *number;
}

// Token 声明
%token INT VOID RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符类型声明
%type <comp_unit> CompUnit
%type <func_def> FuncDef
%type <block> Block
%type <stmt> Stmt
%type <number> Number
%type <str_val> FuncType
%%

CompUnit
  : FuncDef {
    // 创建 CompUnit 节点包装 FuncDef
    $$ = new CompUnit(std::unique_ptr<FuncDef>($1));
    // 将根节点赋值给 ast 参数
    ast = std::unique_ptr<CompUnit>($$);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
FuncDef
  : FuncType IDENT '(' ')' Block {
    // 创建函数定义节点
    bool is_void = false;
    // 这里简化处理：根据 FuncType 判断
    // 实际使用中需要从 $1 获取类型信息
    $$ = new FuncDef(*$2, unique_ptr<Block>($5), is_void);
    delete $1;  // 释放 FuncType 的字符串（当前未使用）
    delete $2;  // 释放 IDENT 字符串
  }
  ;

FuncType
  : INT {
    $$ = new string("int");
  }
  | VOID {
    $$ = new string("void");
  }
  ;

Block
  : '{' Stmt '}' {
    $$ = new Block();
    (*$$).add_stmt(unique_ptr<Stmt>($2));
  }
  ;

Stmt
  : RETURN Number ';' {
    $$ = new Stmt(StmtKind::RETURN);
    (*$$).ret_stmt = make_unique<ReturnStmt>(unique_ptr<Number>($2));
  }
  ;

Number
  : INT_CONST {
    $$ = new Number($1);
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息, parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<CompUnit> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
