#pragma once
#include "../ast/ASTNodes.hpp"
#include "SymbolTable.hpp"
#include <memory>
#include <vector>

class SemanticAnalyzer {
public:
    void analyze(const std::vector<std::shared_ptr<ASTNode>>& nodes);
    void printSymbolTable();

private:
    SymbolTable symbolTable;

    // Visit functions for each node type
    void visit(std::shared_ptr<ASTNode> node);
    void visitTag(std::shared_ptr<TagNode> node);
    void visitVarDecl(std::shared_ptr<VarDeclNode> node);
    void visitAssignment(std::shared_ptr<AssignmentNode> node);
    void visitBinaryOp(std::shared_ptr<BinaryOpNode> node);
    void visitIdentifier(std::shared_ptr<IdentifierNode> node);
    void visitNumber(std::shared_ptr<NumberNode> node);
    void visitString(std::shared_ptr<StringNode> node);
    void visitFunction(std::shared_ptr<FunctionNode> node);
    void visitIf(std::shared_ptr<IfNode> node);
    void visitReturn(std::shared_ptr<ReturnNode> node);
};
