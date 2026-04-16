#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>
#include <iostream>

// AST基类，所有具体的AST节点都会继承

class AstNode {
public:
    virtual ~AstNode() = default;
    virtual void print(std::ostream &os, int indent = 0) const = 0;
};

// 辅助函数：打印缩进
inline void print_indent(std::ostream &os, int indent) {
    for (int i = 0; i < indent; ++i) {
        os << "  ";
    }
};

// 数字节点
class Number : public AstNode{
public:
    int value;
    explicit Number(int val) : value(val) {}

    void print(std::ostream &os, int indent = 0) const override {
        print_indent(os, indent);
        os << "Number: " << value << "\n";
    }
};

/**
 * 返回语句节点
 * 例如: return 5;
 */
class ReturnStmt : public AstNode {
public:
    std::unique_ptr<Number> ret_value;
    
    explicit ReturnStmt(std::unique_ptr<Number> value) 
        : ret_value(std::move(value)) {}

    void print(std::ostream &os, int indent = 0) const override {
        print_indent(os, indent);
        os << "ReturnStmt\n";
        if (ret_value) {
            ret_value->print(os, indent + 1);
        }
    }
};

/**
 * 语句节点（用枚举区分类型，方便扩展）
 */
enum class StmtKind {
    RETURN,
    // 未来扩展: IF, WHILE, ...
};

class Stmt : public AstNode {
public:
    StmtKind kind;
    std::unique_ptr<ReturnStmt> ret_stmt;  // 当 kind == RETURN 时使用
    
    explicit Stmt(StmtKind k) : kind(k) {}

    void print(std::ostream &os, int indent = 0) const override {
        print_indent(os, indent);
        switch (kind) {
            case StmtKind::RETURN:
                os << "Stmt (return)\n";
                if (ret_stmt) {
                    ret_stmt->print(os, indent + 1);
                }
                break;
        }
    }
};

/**
 * 代码块节点
 * 例如: { return 5; }
 */
class Block : public AstNode {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    
    Block() = default;
    void add_stmt(std::unique_ptr<Stmt> stmt) {
        statements.push_back(std::move(stmt));
    }

    void print(std::ostream &os, int indent = 0) const override {
        print_indent(os, indent);
        os << "Block\n";
        for (const auto &stmt : statements) {
            stmt->print(os, indent + 1);
        }
    }
};

/**
 * 函数定义节点
 * 例如: int square() { return 5; }
 */
class FuncDef : public AstNode {
public:
    std::string name;           // 函数名
    std::unique_ptr<Block> body; // 函数体
    // 返回值类型：当前只有 int/void，可以用 bool 或枚举
    bool is_void;               // true: void, false: int
    
    FuncDef(const std::string& n, std::unique_ptr<Block> b, bool isVoid = false)
        : name(n), body(std::move(b)), is_void(isVoid) {}

    void print(std::ostream &os, int indent = 0) const override {
        print_indent(os, indent);
        os << "FuncDef: " << name;
        if (is_void) {
            os << " (void)\n";
        } else {
            os << " (int)\n";
        }
        if (body) {
            body->print(os, indent + 1);
        }
    }
};

/**
 * 编译单元节点（根节点）
 */
class CompUnit : public AstNode {
public:
    std::unique_ptr<FuncDef> func_def;
    
    explicit CompUnit(std::unique_ptr<FuncDef> def) 
        : func_def(std::move(def)) {}

    void print(std::ostream &os, int indent = 0) const override {
        print_indent(os, indent);
        os << "CompUnit\n";
        if (func_def) {
            func_def->print(os, indent + 1);
        }
    }
};

// ============ 创建节点的辅助函数（可选） ============
// 这些函数可以简化 Bison 动作中的代码

inline std::unique_ptr<Number> make_number(int val) {
    return std::make_unique<Number>(val);
}

inline std::unique_ptr<ReturnStmt> make_return_stmt(std::unique_ptr<Number> val) {
    return std::make_unique<ReturnStmt>(std::move(val));
}

inline std::unique_ptr<Stmt> make_return_stmt_as_stmt(std::unique_ptr<Number> val) {
    auto stmt = std::make_unique<Stmt>(StmtKind::RETURN);
    stmt->ret_stmt = std::make_unique<ReturnStmt>(std::move(val));
    return stmt;
}

inline std::unique_ptr<Block> make_block() {
    return std::make_unique<Block>();
}

inline std::unique_ptr<FuncDef> make_func_def(const std::string& name, 
                                               std::unique_ptr<Block> body,
                                               bool is_void = false) {
    return std::make_unique<FuncDef>(name, std::move(body), is_void);
}

#endif // AST_H