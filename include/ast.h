#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>
#include <iostream>

// ============ 前置声明 ============
// 告诉编译器这些类存在，具体定义在后面
class Number;
class ReturnStmt;
class Stmt;
class Block;
class FuncDef;
class CompUnit;

/**
 * Visitor 基类
 * 为每种节点类型声明一个 visit 方法
 * 添加新节点时，需要在这里添加对应的虚函数
 */
class AstVisitor {
public:
    virtual ~AstVisitor() = default;
    
    // 为每种节点类型定义 visit 方法
    virtual void visit(const Number *node) = 0;
    virtual void visit(const ReturnStmt *node) = 0;
    virtual void visit(const Stmt *node) = 0;
    virtual void visit(const Block *node) = 0;
    virtual void visit(const FuncDef *node) = 0;
    virtual void visit(const CompUnit *node) = 0;
};

// AST基类，所有具体的AST节点都会继承

class AstNode {
public:
    virtual ~AstNode() = default;
    virtual void accept(AstVisitor *visitor) const = 0;
};

// 数字节点
class Number : public AstNode{
public:
    int value;
    explicit Number(int val) : value(val) {}

    void accept(AstVisitor *visitor) const override {
        visitor->visit(this);
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

    void accept(AstVisitor *visitor) const override {
        visitor->visit(this);
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

    void accept(AstVisitor *visitor) const override {
        visitor->visit(this);
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

    void accept(AstVisitor *visitor) const override {
        visitor->visit(this);
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

    void accept(AstVisitor *visitor) const override {
        visitor->visit(this);
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

    void accept(AstVisitor *visitor) const override {
        visitor->visit(this);
    }
};

// ============ 具体的 Visitor 实现示例 ============

/**
 * 打印 Visitor：遍历 AST 并打印结构
 */
class PrintVisitor : public AstVisitor {
private:
    std::ostream &os;
    
public:
    explicit PrintVisitor(std::ostream &out) : os(out) {}
    
    void visit(const Number *node) override {
        os << node->value;
    }
    
    void visit(const ReturnStmt *node) override {
        os << "ReturnStmt { ";
        if (node->ret_value) {
            node->ret_value->accept(this);
        }
        os << " }";
    }
    
    void visit(const Stmt *node) override {
        switch (node->kind) {
            case StmtKind::RETURN:
                if (node->ret_stmt) {
                    node->ret_stmt->accept(this);
                }
                break;
        }
    }
    
    void visit(const Block *node) override {
        os << "Block { ";
        for (const auto &stmt : node->statements) {
            stmt->accept(this);
        }
        os << " }";
    }
    
    void visit(const FuncDef *node) override {
        os << "FuncDef { ";
        os << "FuncType ";
        if (node->is_void) {
            os << "{ void }";
        } else {
            os << "{ int }";
        }
        os <<", "<< node->name;
        if (node->body) {
            os << ", ";
            node->body->accept(this);
        }
        os << " }";
    }
    
    void visit(const CompUnit *node) override {
        os << "CompUnit { ";
        if (node->func_def) {
            node->func_def->accept(this);
        }
        os << " }\n";
    }
};

class KoopaVisitor : public AstVisitor {
    std::ostream &os;
public:
    explicit KoopaVisitor(std::ostream &out) : os(out) {}

    void visit(const Number *node) override {
        os << node->value;  // 输出数字
    }
    
    void visit(const ReturnStmt *node) override {
        os << "  ret ";
        node->ret_value->accept(this);
        os << "\n";
    }

    void visit(const Stmt *node) override {
        switch (node->kind) {
            case StmtKind::RETURN:
                if (node->ret_stmt) {
                    node->ret_stmt->accept(this);
                }
                break;
        }
    }
    
    void visit(const FuncDef *node) override {
        os << "fun @" << node->name << "(): i32 {\n";
        node->body->accept(this);
        os << "}\n";
    }

    void visit(const Block *node) override {
        os << "%entry:\n";
        for (const auto &stmt : node->statements) {
            stmt->accept(this);
        }
    }

    void visit(const CompUnit *node) override {
        if (node->func_def) {
            node->func_def->accept(this);
        }
    }
    // ... 其他节点
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