#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstdio>

// Base class for all nodes in the tree
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void debugPrint(int indent) = 0; // Helper to visualize the tree
};

// 1. HTML Element Node (e.g., <div id="app">)
class TagNode : public ASTNode {
public:
    std::string tagName;
    std::string id;
    std::string innerText;
    std::vector<std::shared_ptr<ASTNode>> children;

    TagNode(std::string name, std::string id_val) 
        : tagName(name), id(id_val) {}

    void debugPrint(int indent) override {
        for(int i=0; i<indent; i++) printf("  ");
        printf("HTML TAG: <%s> ID: [%s]\n", tagName.c_str(), id.c_str());
        for(auto& child : children) child->debugPrint(indent + 1);
    }
};

// 2. Number Node (e.g., 10, 5.5)
class NumberNode : public ASTNode {
public:
    double value;
    NumberNode(double val) : value(val) {}

    void debugPrint(int indent) override {
        for(int i=0; i<indent; i++) printf("  ");
        printf("JS NUMBER: %f\n", value);
    }
};

// 3. Binary Operation Node (e.g., x + y)
class BinaryOpNode : public ASTNode {
public:
    char op;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;

    BinaryOpNode(char op, std::shared_ptr<ASTNode> lhs, std::shared_ptr<ASTNode> rhs)
        : op(op), left(lhs), right(rhs) {}

    void debugPrint(int indent) override {
        for(int i=0; i<indent; i++) printf("  ");
        printf("JS BINARY OP: (%c)\n", op);
        left->debugPrint(indent + 1);
        right->debugPrint(indent + 1);
    }
};

// 4. Variable Declaration Node (e.g., let x = 10)
class VarDeclNode : public ASTNode {
public:
    std::string varName;
    std::shared_ptr<ASTNode> initValue;

    VarDeclNode(std::string name, std::shared_ptr<ASTNode> val)
        : varName(name), initValue(val) {}

    void debugPrint(int indent) override {
        for(int i=0; i<indent; i++) printf("  ");
        printf("JS VAR DECL: %s\n", varName.c_str());
        if(initValue) initValue->debugPrint(indent + 1);
    }
};

// 5. Identifier Node (e.g., variable reference like "x" or "count")
class IdentifierNode : public ASTNode {
public:
    std::string name;
    IdentifierNode(std::string n) : name(n) {}

    void debugPrint(int indent) override {
        for(int i=0; i<indent; i++) printf("  ");
        printf("JS IDENTIFIER: %s\n", name.c_str());
    }
};

// 6. String Literal Node (e.g., "hello world")
class StringNode : public ASTNode {
public:
    std::string value;
    StringNode(std::string val) : value(val) {}

    void debugPrint(int indent) override {
        for(int i=0; i<indent; i++) printf("  ");
        printf("JS STRING: \"%s\"\n", value.c_str());
    }
};

// 7. Function Declaration Node (e.g., function add(a, b) { ... })
class FunctionNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::shared_ptr<ASTNode>> body;

    FunctionNode(std::string n) : name(n) {}

    void debugPrint(int indent) override {
        for(int i=0; i<indent; i++) printf("  ");
        printf("JS FUNCTION: %s(", name.c_str());
        for(size_t i = 0; i < parameters.size(); i++) {
            printf("%s", parameters[i].c_str());
            if(i < parameters.size() - 1) printf(", ");
        }
        printf(")\n");
        for(auto& stmt : body) stmt->debugPrint(indent + 1);
    }
};

// 8. If Statement Node (e.g., if (condition) { ... } else { ... })
class IfNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> thenBranch;
    std::vector<std::shared_ptr<ASTNode>> elseBranch;

    IfNode(std::shared_ptr<ASTNode> cond) : condition(cond) {}

    void debugPrint(int indent) override {
        for(int i=0; i<indent; i++) printf("  ");
        printf("JS IF STATEMENT:\n");
        for(int i=0; i<indent+1; i++) printf("  ");
        printf("Condition:\n");
        condition->debugPrint(indent + 2);
        for(int i=0; i<indent+1; i++) printf("  ");
        printf("Then:\n");
        for(auto& stmt : thenBranch) stmt->debugPrint(indent + 2);
        if(!elseBranch.empty()) {
            for(int i=0; i<indent+1; i++) printf("  ");
            printf("Else:\n");
            for(auto& stmt : elseBranch) stmt->debugPrint(indent + 2);
        }
    }
};

// 9. Return Statement Node (e.g., return x + 5;)
class ReturnNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> value;

    ReturnNode(std::shared_ptr<ASTNode> val) : value(val) {}

    void debugPrint(int indent) override {
        for(int i=0; i<indent; i++) printf("  ");
        printf("JS RETURN:\n");
        if(value) value->debugPrint(indent + 1);
    }
};

// 10. Assignment Node (e.g., x = 10)
class AssignmentNode : public ASTNode {
public:
    std::string varName;
    std::shared_ptr<ASTNode> value;

    AssignmentNode(std::string name, std::shared_ptr<ASTNode> val)
        : varName(name), value(val) {}

    void debugPrint(int indent) override {
        for(int i=0; i<indent; i++) printf("  ");
        printf("JS ASSIGNMENT: %s =\n", varName.c_str());
        value->debugPrint(indent + 1);
    }
};
