#include "SemanticAnalyzer.hpp"
#include <iostream>

void SemanticAnalyzer::analyze(const std::vector<std::shared_ptr<ASTNode>>& nodes) {
    std::cout << "\n[Semantic Analysis] Starting analysis...\n\n";
    
    for (const auto& node : nodes) {
        visit(node);
    }
    
    std::cout << "\n[Semantic Analysis] Analysis complete!\n";
}

void SemanticAnalyzer::printSymbolTable() {
    symbolTable.printAllSymbols();
}

void SemanticAnalyzer::visit(std::shared_ptr<ASTNode> node) {
    if (auto tag = std::dynamic_pointer_cast<TagNode>(node)) {
        visitTag(tag);
    }
    else if (auto varDecl = std::dynamic_pointer_cast<VarDeclNode>(node)) {
        visitVarDecl(varDecl);
    }
    else if (auto assignment = std::dynamic_pointer_cast<AssignmentNode>(node)) {
        visitAssignment(assignment);
    }
    else if (auto binaryOp = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        visitBinaryOp(binaryOp);
    }
    else if (auto identifier = std::dynamic_pointer_cast<IdentifierNode>(node)) {
        visitIdentifier(identifier);
    }
    else if (auto number = std::dynamic_pointer_cast<NumberNode>(node)) {
        visitNumber(number);
    }
    else if (auto str = std::dynamic_pointer_cast<StringNode>(node)) {
        visitString(str);
    }
    else if (auto func = std::dynamic_pointer_cast<FunctionNode>(node)) {
        visitFunction(func);
    }
    else if (auto ifNode = std::dynamic_pointer_cast<IfNode>(node)) {
        visitIf(ifNode);
    }
    else if (auto forNode = std::dynamic_pointer_cast<ForNode>(node)) {
        visitFor(forNode);
    }
    else if (auto whileNode = std::dynamic_pointer_cast<WhileNode>(node)) {
        visitWhile(whileNode);
    }
    else if (auto returnNode = std::dynamic_pointer_cast<ReturnNode>(node)) {
        visitReturn(returnNode);
    }
}

void SemanticAnalyzer::visitTag(std::shared_ptr<TagNode> node) {
    std::cout << "[Semantics] Processing HTML Tag: <" << node->tagName << ">\n";
    
    if (!node->id.empty()) {
        std::cout << "[Semantics] ✓ Registering HTML Element ID: #" << node->id << "\n";
        symbolTable.define(node->id, SymbolType::HTML_ELEMENT, 0);
    }

    for (auto& child : node->children) {
        visit(child);
    }
}

void SemanticAnalyzer::visitVarDecl(std::shared_ptr<VarDeclNode> node) {
    std::cout << "[Semantics] Processing Variable Declaration: " << node->varName << "\n";
    
    if (node->initValue) {
        visit(node->initValue);
    }
    

    symbolTable.define(node->varName, SymbolType::NUMBER, 0);
    std::cout << "[Semantics] ✓ Defined JS Variable: " << node->varName << "\n";
}

void SemanticAnalyzer::visitAssignment(std::shared_ptr<AssignmentNode> node) {
    std::cout << "[Semantics] Processing Assignment: " << node->varName << "\n";
    
    if (!symbolTable.exists(node->varName)) {
        throw std::runtime_error("Semantic Error: Cannot assign to undefined variable '" + 
                               node->varName + "'");
    }
    
    Symbol sym = symbolTable.lookup(node->varName);
    if (sym.isConst) {
        throw std::runtime_error("Semantic Error: Cannot assign to const variable '" + 
                               node->varName + "'");
    }
    
    if (node->value) {
        visit(node->value);
    }
    
    std::cout << "[Semantics] ✓ Assignment to existing variable: " << node->varName << "\n";
}

void SemanticAnalyzer::visitBinaryOp(std::shared_ptr<BinaryOpNode> node) {
    if (node->left) {
        visit(node->left);
    }
    if (node->right) {
        visit(node->right);
    }
    
}

void SemanticAnalyzer::visitIdentifier(std::shared_ptr<IdentifierNode> node) {
    std::cout << "[Semantics] Checking identifier: " << node->name << "\n";
    
    if (!symbolTable.exists(node->name)) {
        throw std::runtime_error("Semantic Error: Undefined identifier '" + 
                               node->name + "'");
    }
    
    Symbol sym = symbolTable.lookup(node->name);
    std::cout << "[Semantics] ✓ Identifier '" << node->name << "' is defined\n";
}

void SemanticAnalyzer::visitNumber(std::shared_ptr<NumberNode> node) {
}

void SemanticAnalyzer::visitString(std::shared_ptr<StringNode> node) {
}

void SemanticAnalyzer::visitFunction(std::shared_ptr<FunctionNode> node) {
    std::cout << "[Semantics] Processing Function: " << node->name << "\n";
    
    symbolTable.define(node->name, SymbolType::FUNCTION, 0);
    
    symbolTable.pushScope();
    
    for (const auto& param : node->parameters) {
        symbolTable.define(param, SymbolType::NUMBER, 0);
        std::cout << "[Semantics] ✓ Defined parameter: " << param << "\n";
    }
    
    for (auto& stmt : node->body) {
        visit(stmt);
    }
    
    symbolTable.popScope();
    
    std::cout << "[Semantics] ✓ Function '" << node->name << "' analyzed\n";
}

void SemanticAnalyzer::visitIf(std::shared_ptr<IfNode> node) {
    std::cout << "[Semantics] Processing If Statement\n";
    
    if (node->condition) {
        visit(node->condition);
    }
    
    symbolTable.pushScope();
    for (auto& stmt : node->thenBranch) {
        visit(stmt);
    }
    symbolTable.popScope();
    
    if (!node->elseBranch.empty()) {
        symbolTable.pushScope();
        for (auto& stmt : node->elseBranch) {
            visit(stmt);
        }
        symbolTable.popScope();
    }
}

void SemanticAnalyzer::visitReturn(std::shared_ptr<ReturnNode> node) {
    std::cout << "[Semantics] Processing Return Statement\n";
    if (node->value) {
        visit(node->value);
    }
}

void SemanticAnalyzer::visitFor(std::shared_ptr<ForNode> node) {
    std::cout << "[Semantics] Processing For Loop\n";
    if (node->init) visit(node->init);
    if (node->condition) visit(node->condition);
    symbolTable.pushScope();
    for (auto& stmt : node->body) visit(stmt);
    symbolTable.popScope();
    if (node->increment) visit(node->increment);
}

void SemanticAnalyzer::visitWhile(std::shared_ptr<WhileNode> node) {
    std::cout << "[Semantics] Processing While Loop\n";
    if (node->condition) visit(node->condition);
    symbolTable.pushScope();
    for (auto& stmt : node->body) visit(stmt);
    symbolTable.popScope();
}
