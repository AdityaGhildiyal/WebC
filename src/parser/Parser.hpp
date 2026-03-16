#pragma once
#include "../lexer/Lexer.hpp"
#include "../ast/ASTNodes.hpp"
#include <vector>
#include <memory>
#include <stdexcept>

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::shared_ptr<ASTNode>> parseProgram();

private:
    std::vector<Token> tokens;
    size_t pos = 0;

    // Helper functions
    Token peek();
    Token advance();
    bool match(TokenType type);
    Token expect(TokenType type, const std::string& message);
    bool isAtEnd();

    // Grammar rules
    std::shared_ptr<ASTNode> parseNode();
    std::shared_ptr<ASTNode> parseTag();
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseVarDecl();
    std::shared_ptr<ASTNode> parseAssignment();
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseComparison();
    std::shared_ptr<ASTNode> parseTerm();        // Multiplication/Division
    std::shared_ptr<ASTNode> parseFactor();      // Numbers/Parentheses/Identifiers
    std::shared_ptr<ASTNode> parsePrimary();     // Literals and identifiers
};
