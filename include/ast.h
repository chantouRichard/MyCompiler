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
class Exp;
class ParenExp;
class UnaryExpression;
class BinaryExpression;

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
    virtual void visit(const ParenExp *node) = 0;
    virtual void visit(const UnaryExpression *node) = 0;
    virtual void visit(const BinaryExpression *node) = 0;
};

// AST基类，所有具体的AST节点都会继承

class AstNode {
public:
    virtual ~AstNode() = default;
    virtual void accept(AstVisitor *visitor) const = 0;
};

// 表达式基类
class Exp : public AstNode {
public:
    virtual ~Exp() = default;
};

// 数字节点（属于 PrimaryExp）
class Number : public Exp {
public:
    int value;
    explicit Number(int val) : value(val) {}

    void accept(AstVisitor *visitor) const override {
        visitor->visit(this);
    }
};

// 括号表达式节点 ( (Exp) )
class ParenExp : public Exp {
public:
    std::unique_ptr<Exp> expr;
    explicit ParenExp(std::unique_ptr<Exp> e) : expr(std::move(e)) {}

    void accept(AstVisitor *visitor) const override {
        visitor->visit(this);
    }
};

enum class UnaryOp {
    PLUS,   // +
    MINUS,  // -
    NOT     // !
};

class UnaryExpression : public Exp {
public:
    UnaryOp op;
    std::unique_ptr<Exp> operand;

    UnaryExpression(UnaryOp o, std::unique_ptr<Exp> od) 
        : op(o), operand(std::move(od)) {}

    void accept(AstVisitor *visitor) const override {
        visitor->visit(this);
    }
};

enum class BinaryOp {
    // 算术运算符
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    
    // 关系运算符 (RelExp)
    LT,     // <
    GT,     // >
    LE,     // <=
    GE,     // >=
    
    // 相等运算符 (EqExp)
    EQ,     // ==
    NE,     // !=
    
    // 逻辑运算符
    AND,    // &&
    OR      // ||
};

class BinaryExpression : public Exp {
public:
    BinaryOp op;
    std::unique_ptr<Exp> left;
    std::unique_ptr<Exp> right;

    BinaryExpression(BinaryOp o, std::unique_ptr<Exp> l, std::unique_ptr<Exp> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}

    void accept(AstVisitor *visitor) const override {
        visitor->visit(this);
    }
};

/**
 * 返回语句节点
 * 例如: return -(5 + !0);
 */
class ReturnStmt : public AstNode {
public:
    std::unique_ptr<Exp> exp; // 现在指向通用的表达式
    
    explicit ReturnStmt(std::unique_ptr<Exp> e) 
        : exp(std::move(e)) {}

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
    // 使用统一的基类指针，增强扩展性
    std::unique_ptr<AstNode> stmt_entity; 
    
    explicit Stmt(StmtKind k, std::unique_ptr<AstNode> entity) 
        : kind(k), stmt_entity(std::move(entity)) {}

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

    void visit(const ParenExp *node) override {
        os << "ParenExp { ";
        node->expr->accept(this);
        os << " }";
    }

    void visit(const UnaryExpression *node) override {
        os << "UnaryExpression { op: ";
        switch(node->op) {
            case UnaryOp::PLUS:  os << "+"; break;
            case UnaryOp::MINUS: os << "-"; break;
            case UnaryOp::NOT:   os << "!"; break;
        }
        os << ", operand: ";
        node->operand->accept(this); // 注意这里是 operand
        os << " }";
    }

    void visit(const BinaryExpression *node) override {
        os << "BinaryExpression { op: ";
        switch(node->op) {
            case BinaryOp::ADD:  os << "+"; break;
            case BinaryOp::SUB:  os << "-"; break;
            case BinaryOp::MUL:  os << "*"; break;
            case BinaryOp::DIV:  os << "/"; break;
            case BinaryOp::MOD:  os << "%"; break;
            case BinaryOp::LT:   os << "<"; break;
            case BinaryOp::GT:   os << ">"; break;
            case BinaryOp::LE:   os << "<="; break;
            case BinaryOp::GE:   os << ">="; break;
            case BinaryOp::EQ:   os << "=="; break;
            case BinaryOp::NE:   os << "!="; break;
            case BinaryOp::AND:  os << "&&"; break;
            case BinaryOp::OR:   os << "||"; break;
        }
        os << ", left: ";
        node->left->accept(this);
        os << " }";
        os << ", right: ";
        node->right->accept(this);
        os << " }";
    }
    
    void visit(const ReturnStmt *node) override {
        os << "ReturnStmt { ";
        if (node->exp) {
            node->exp->accept(this);
        }
        os << " }";
    }
    
    void visit(const Stmt *node) override {
        switch (node->kind) {
            case StmtKind::RETURN:
                if (node->stmt_entity) {
                    node->stmt_entity->accept(this);
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
    mutable int temp_count = 0;

    // 辅助函数：生成并返回下一个临时变量名
    std::string next_temp() const {
        return "%" + std::to_string(temp_count++);
    }

    // 假设我们需要记录上一次表达式计算的结果变量名
    mutable std::string last_res;

public:
    explicit KoopaVisitor(std::ostream &out) : os(out) {}

    void visit(const Number *node) override {
        last_res = std::to_string(node->value);
    }

    void visit(const ParenExp *node) override {
        // 括号表达式直接处理内部表达式即可
        node->expr->accept(this);
    }

    void visit(const UnaryExpression *node) override {
        // 1. 先递归处理操作数
        node->operand->accept(this);
        std::string src = last_res; // 获取操作数的结果（可能是数字，也可能是临时变量）

        // 2. 生成当前运算的目标临时变量
        std::string dest = next_temp();
        
        // 3. 根据运算符输出指令
        switch (node->op) {
            case UnaryOp::PLUS:
                // 正号通常不产生指令，直接传递结果即可
                // 或者生成一条 add %dest, 0, %src
                last_res = src;
                return; 
            case UnaryOp::MINUS:
                os << "  " << dest << " = sub 0, " << src << "\n";
                break;
            case UnaryOp::NOT:
                os << "  " << dest << " = eq " << src << ", 0\n";
                break;
        }
        last_res = dest; // 更新最后一次计算的结果为当前的临时变量
    }

    void visit(const BinaryExpression *node) override {
        // 1. 先递归处理操作数
        node->left->accept(this);
        std::string left_val = last_res; // 关键：立即保存左侧结果
        node->right->accept(this);
        std::string right_val = last_res; // 保存右侧结果

        // 2. 生成当前运算的目标临时变量
        std::string dest = next_temp();
        
        // 根据 koopa.h，二元操作包含 add, sub, mul, div, mod 等
        switch (node->op) {
            case BinaryOp::ADD:
                os << "  " << dest << " = add " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::SUB:
                os << "  " << dest << " = sub " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::MUL:
                os << "  " << dest << " = mul " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::DIV:
                os << "  " << dest << " = div " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::MOD:
                os << "  " << dest << " = mod " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::LT:
                os << "  " << dest << " = lt " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::GT:
                os << "  " << dest << " = gt " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::LE:
                os << "  " << dest << " = le " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::GE:
                os << "  " << dest << " = ge " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::EQ:
                os << "  " << dest << " = eq " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::NE:
                os << "  " << dest << " = ne " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::AND:
                os << "  " << dest << " = and " << left_val << ", " << right_val << "\n";
                break;
            case BinaryOp::OR:
                os << "  " << dest << " = or " << left_val << ", " << right_val << "\n";
                break;
            // 如果你的二元运算还包含比较运算（如 eq, ne, lt, gt 等）
            // 可以继续在此扩展
        }
        last_res = dest; // 更新最后一次计算的结果为当前的临时变量
    }
    
    void visit(const ReturnStmt *node) override {
        node->exp->accept(this);
        os << "  ret " << last_res << "\n";
    }

    void visit(const Stmt *node) override {
        switch (node->kind) {
            case StmtKind::RETURN:
                if (node->stmt_entity) {
                    node->stmt_entity->accept(this);
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

inline std::unique_ptr<ReturnStmt> make_return_stmt(std::unique_ptr<Exp> val) {
    return std::make_unique<ReturnStmt>(std::move(val));
}

inline std::unique_ptr<Stmt> make_return_stmt_as_stmt(std::unique_ptr<Exp> val) {
    auto stmt = std::make_unique<Stmt>(StmtKind::RETURN, std::make_unique<ReturnStmt>(std::move(val)));
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