#pragma once
#include <string>
#include <vector>
#include <memory>

class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class TagNode : public ASTNode {
public:
    std::string tagName;
    std::string id;
    std::string innerText;
    std::vector<std::shared_ptr<ASTNode>> children;

    TagNode(std::string name, std::string id_val) 
        : tagName(name), id(id_val) {}
};

class NumberNode : public ASTNode {
public:
    double value;
    NumberNode(double val) : value(val) {}
};

class BinaryOpNode : public ASTNode {
public:
    char op;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;

    BinaryOpNode(char op, std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> rhs)
        : op(op), left(lhs), right(rhs) {}
};

class VarDeclNode : public ASTNode {
public:
    std::string varName;
    std::shared_ptr<ASTNode> initValue;

    VarDeclNode(std::string name, std::shared_ptr<ASTNode> val)
        : varName(name), initValue(val) {}
};

class IdentifierNode : public ASTNode {
public:
    std::string name;
    IdentifierNode(std::string n) : name(n) {}
};

class StringNode : public ASTNode {
public:
    std::string value;
    StringNode(std::string val) : value(val) {}
};

class FunctionNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::shared_ptr<ASTNode>> body;

    FunctionNode(std::string n) : name(n) {}
};

class IfNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> thenBranch;
    std::vector<std::shared_ptr<ASTNode>> elseBranch;

    IfNode(std::shared_ptr<ASTNode> cond) : condition(cond) {}
};

class ReturnNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> value;

    ReturnNode(std::shared_ptr<ASTNode> val) : value(val) {}
};

class AssignmentNode : public ASTNode {
public:
    std::string varName;
    std::shared_ptr<ASTNode> value;

    AssignmentNode(std::string name, std::shared_ptr<ASTNode> val)
        : varName(name), value(val) {}
};

class ForNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> init;       
    std::shared_ptr<ASTNode> condition;  
    std::shared_ptr<ASTNode> increment;  
    std::vector<std::shared_ptr<ASTNode>> body;

    ForNode(std::shared_ptr<ASTNode> i,
            std::shared_ptr<ASTNode> cond,
            std::shared_ptr<ASTNode> inc)
        : init(i), condition(cond), increment(inc) {}
};

class WhileNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> body;

    WhileNode(std::shared_ptr<ASTNode> cond) : condition(cond) {}
};
