#include "Parser.hpp"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

Token Parser::peek() { 
    return tokens[pos]; 
}

bool Parser::isAtEnd() { 
    return peek().type == TokenType::EOF_TOKEN; 
}

Token Parser::advance() { 
    if (!isAtEnd()) pos++; 
    return tokens[pos - 1]; 
}

bool Parser::match(TokenType type) {
    if (peek().type == type) { 
        advance(); 
        return true; 
    }
    return false;
}

Token Parser::expect(TokenType type, const std::string& message) {
    if (peek().type == type) return advance();
    throw std::runtime_error("Parser Error: " + message + 
                            " at line " + std::to_string(peek().line) + 
                            ", column " + std::to_string(peek().column));
}

std::vector<std::shared_ptr<ASTNode>> Parser::parseProgram() {
    std::vector<std::shared_ptr<ASTNode>> nodes;
    while (!isAtEnd()) {
        nodes.push_back(parseNode());
    }
    return nodes;
}

std::shared_ptr<ASTNode> Parser::parseNode() {
    if (peek().type == TokenType::TAG_OPEN) {
        return parseTag();
    }
    if (peek().type == TokenType::IF) {
        return parseIfStatement();
    }
    if (peek().type == TokenType::FOR) {
        return parseForStatement();
    }
    if (peek().type == TokenType::WHILE) {
        return parseWhileStatement();
    }
    if (peek().type == TokenType::LET || peek().type == TokenType::CONST) {
        return parseStatement();
    }
    if (peek().type == TokenType::IDENTIFIER &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1].type == TokenType::EQUALS) {
        return parseStatement();
    }
    return parseExpression();
}

std::shared_ptr<ASTNode> Parser::parseTag() {
    expect(TokenType::TAG_OPEN, "Expected '<'");
    std::string name = expect(TokenType::IDENTIFIER, "Expected tag name").value;
     
    std::string id = "";
    while (!isAtEnd() && peek().type == TokenType::IDENTIFIER) {
        std::string attrName = advance().value;
        if (peek().type == TokenType::EQUALS) {
            advance(); 
            if (peek().type == TokenType::STRING) {
                std::string val = advance().value;
                if (attrName == "id") id = val;
            } else if (peek().type == TokenType::NUMBER || peek().type == TokenType::IDENTIFIER) {
                advance(); 
            }
        }
    }

    expect(TokenType::TAG_CLOSE, "Expected '>'");
    auto node = std::make_shared<TagNode>(name, id);

    while (!isAtEnd() && 
           !(peek().type == TokenType::TAG_OPEN && 
             pos + 1 < tokens.size() && 
             tokens[pos + 1].type == TokenType::SLASH)) {
        node->children.push_back(parseNode());
    }

    expect(TokenType::TAG_OPEN, "Expected '</'");
    expect(TokenType::SLASH, "Expected '/'");
    std::string closingName = expect(TokenType::IDENTIFIER, "Expected closing tag name").value;
    if (closingName != name) {
        throw std::runtime_error(
            "Mismatched tag: opened <" + name + "> but closed </" + closingName + ">"
        );
    }
    expect(TokenType::TAG_CLOSE, "Expected '>'");

    return node;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (peek().type == TokenType::LET || peek().type == TokenType::CONST) {
        return parseVarDecl();
    }
    if (peek().type == TokenType::IDENTIFIER) {
        if (pos + 1 < tokens.size() && tokens[pos + 1].type == TokenType::EQUALS) {
            return parseAssignment();
        }
    }
    return parseExpression();
}

std::shared_ptr<ASTNode> Parser::parseVarDecl() {
    advance(); 
    std::string name = expect(TokenType::IDENTIFIER, "Expected variable name").value;
    expect(TokenType::EQUALS, "Expected '='");
    auto value = parseExpression();
    expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_shared<VarDeclNode>(name, value);
}

std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    advance(); 
    expect(TokenType::LPAREN, "Expected '(' after 'if'");
    auto cond = parseExpression();
    expect(TokenType::RPAREN, "Expected ')' after if condition");

    expect(TokenType::LBRACE, "Expected '{' after if condition");
    auto ifNode = std::make_shared<IfNode>(cond);
    while (!isAtEnd() && peek().type != TokenType::RBRACE) {
        ifNode->thenBranch.push_back(parseNode());
    }
    expect(TokenType::RBRACE, "Expected '}' to close if block");

    if (!isAtEnd() && peek().type == TokenType::ELSE) {
        advance();
        expect(TokenType::LBRACE, "Expected '{' after 'else'");
        while (!isAtEnd() && peek().type != TokenType::RBRACE) {
            ifNode->elseBranch.push_back(parseNode());
        }
        expect(TokenType::RBRACE, "Expected '}' to close else block");
    }

    return ifNode;
}

std::shared_ptr<ASTNode> Parser::parseForStatement() {
    advance(); 
    expect(TokenType::LPAREN, "Expected '(' after 'for'");

    std::shared_ptr<ASTNode> init;
    if (peek().type == TokenType::LET || peek().type == TokenType::CONST) {
        init = parseVarDecl(); 
    } else {
        init = parseAssignment(); 
    }

    auto cond = parseExpression();
    expect(TokenType::SEMICOLON, "Expected ';' after for-condition");

    std::shared_ptr<ASTNode> inc;
    if (peek().type == TokenType::IDENTIFIER &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1].type == TokenType::EQUALS) {
        std::string name = advance().value;
        advance();                          
        auto val = parseExpression();
        inc = std::make_shared<AssignmentNode>(name, val);
    }

    expect(TokenType::RPAREN, "Expected ')' to close for header");

    expect(TokenType::LBRACE, "Expected '{' to open for body");
    auto forNode = std::make_shared<ForNode>(init, cond, inc);
    while (!isAtEnd() && peek().type != TokenType::RBRACE) {
        forNode->body.push_back(parseNode());
    }
    expect(TokenType::RBRACE, "Expected '}' to close for body");

    return forNode;
}

std::shared_ptr<ASTNode> Parser::parseWhileStatement() {
    advance();
    expect(TokenType::LPAREN, "Expected '(' after 'while'");
    auto cond = parseExpression();
    expect(TokenType::RPAREN, "Expected ')' after while condition");

    expect(TokenType::LBRACE, "Expected '{' to open while body");
    auto whileNode = std::make_shared<WhileNode>(cond);
    while (!isAtEnd() && peek().type != TokenType::RBRACE) {
        whileNode->body.push_back(parseNode());
    }
    expect(TokenType::RBRACE, "Expected '}' to close while body");

    return whileNode;
}

std::shared_ptr<ASTNode> Parser::parseAssignment() {
    std::string name = expect(TokenType::IDENTIFIER, "Expected variable name").value;
    expect(TokenType::EQUALS, "Expected '='");
    auto value = parseExpression();
    expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_shared<AssignmentNode>(name, value);
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseComparison();
    
    while (peek().type == TokenType::AND || peek().type == TokenType::OR) {
        char op = (peek().type == TokenType::AND) ? '&' : '|';
        advance();
        auto right = parseComparison();
        left = std::make_shared<BinaryOpNode>(op, left, right);
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseComparison() {
    auto left = parseTerm();
    

    while (peek().type == TokenType::DOUBLE_EQUALS ||
           peek().type == TokenType::NOT_EQUALS     ||
           peek().type == TokenType::LESS_THAN      ||
           peek().type == TokenType::GREATER_THAN   ||
           peek().type == TokenType::LESS_EQUALS    ||
           peek().type == TokenType::GREATER_EQUALS) {

        char op;
        if      (peek().type == TokenType::DOUBLE_EQUALS)  { op = '='; advance(); }
        else if (peek().type == TokenType::NOT_EQUALS)      { op = '!'; advance(); }
        else if (peek().type == TokenType::LESS_THAN)       { op = '<'; advance(); }
        else if (peek().type == TokenType::GREATER_THAN)    { op = '>'; advance(); }
        else if (peek().type == TokenType::LESS_EQUALS)     { op = 'L'; advance(); }
        else if (peek().type == TokenType::GREATER_EQUALS)  { op = 'G'; advance(); }
        else break;

        auto right = parseTerm();
        left = std::make_shared<BinaryOpNode>(op, left, right);
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();
    
    while (peek().type == TokenType::PLUS || peek().type == TokenType::MINUS) {
        char op = peek().value[0];
        advance();
        auto right = parseFactor();
        left = std::make_shared<BinaryOpNode>(op, left, right);
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseFactor() {
    auto left = parsePrimary();
    
    while (peek().type == TokenType::ASTERISK || peek().type == TokenType::SLASH) {
        char op = peek().value[0];
        advance();
        auto right = parsePrimary();
        left = std::make_shared<BinaryOpNode>(op, left, right);
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parsePrimary() {
    if (peek().type == TokenType::NUMBER) {
        double value = std::stod(advance().value);
        return std::make_shared<NumberNode>(value);
    }
    
    if (peek().type == TokenType::STRING) {
        std::string value = advance().value;
        return std::make_shared<StringNode>(value);
    }
    
    if (peek().type == TokenType::IDENTIFIER) {
        std::string name = advance().value;
        return std::make_shared<IdentifierNode>(name);
    }
    
    if (peek().type == TokenType::LPAREN) {
        advance(); 
        auto expr = parseExpression();
        expect(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    throw std::runtime_error("Unexpected token in expression at line " + 
                            std::to_string(peek().line));
}
