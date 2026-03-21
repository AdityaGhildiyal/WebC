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

// Entry point: Program -> (Tag | Statement)*
std::vector<std::shared_ptr<ASTNode>> Parser::parseProgram() {
    std::vector<std::shared_ptr<ASTNode>> nodes;
    while (!isAtEnd()) {
        nodes.push_back(parseNode());
    }
    return nodes;
}

std::shared_ptr<ASTNode> Parser::parseNode() {
    // HTML tag
    if (peek().type == TokenType::TAG_OPEN) {
        return parseTag();
    }
    // if statement
    if (peek().type == TokenType::IF) {
        return parseIfStatement();
    }
    // for loop
    if (peek().type == TokenType::FOR) {
        return parseForStatement();
    }
    // Statement (let, const, assignment)
    if (peek().type == TokenType::LET || peek().type == TokenType::CONST) {
        return parseStatement();
    }
    // Bare assignment: IDENTIFIER =
    if (peek().type == TokenType::IDENTIFIER &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1].type == TokenType::EQUALS) {
        return parseStatement();
    }
    // Default to expression
    return parseExpression();
}

// Tag -> < IDENTIFIER (id = STRING)? > (Node)* < / IDENTIFIER >
std::shared_ptr<ASTNode> Parser::parseTag() {
    expect(TokenType::TAG_OPEN, "Expected '<'");
    std::string name = expect(TokenType::IDENTIFIER, "Expected tag name").value;
    
    std::string id = "";
    // Parse all attributes: extract 'id', skip everything else
    while (!isAtEnd() && peek().type == TokenType::IDENTIFIER) {
        std::string attrName = advance().value; // consume attribute name
        if (peek().type == TokenType::EQUALS) {
            advance(); // consume '='
            if (peek().type == TokenType::STRING) {
                std::string val = advance().value;
                if (attrName == "id") id = val;
                // other attributes (class, href, lang, style…) are ignored in WebC
            } else if (peek().type == TokenType::NUMBER || peek().type == TokenType::IDENTIFIER) {
                advance(); // skip bare-word / numeric value
            }
        }
        // boolean attribute (no '=') — name already consumed, continue
    }

    expect(TokenType::TAG_CLOSE, "Expected '>'");
    auto node = std::make_shared<TagNode>(name, id);

    // Parse children until we hit </
    while (!isAtEnd() && 
           !(peek().type == TokenType::TAG_OPEN && 
             pos + 1 < tokens.size() && 
             tokens[pos + 1].type == TokenType::SLASH)) {
        node->children.push_back(parseNode());
    }

    // Closing tag
    expect(TokenType::TAG_OPEN, "Expected '</'");
    expect(TokenType::SLASH, "Expected '/'");
    expect(TokenType::IDENTIFIER, "Expected closing tag name");
    expect(TokenType::TAG_CLOSE, "Expected '>'");

    return node;
}

// ─── Helper: parse a brace-delimited block of nodes ────────────────────────
// Expects the leading '{' to still be the current token.
static std::vector<std::shared_ptr<ASTNode>> parseBlock(Parser& p,
    std::function<std::shared_ptr<ASTNode>()> parseNodeFn,
    std::function<bool()> isAtEndFn,
    std::function<Token()> peekFn,
    std::function<Token()> advanceFn)
{
    (void)p;
    advanceFn(); // consume '{'
    std::vector<std::shared_ptr<ASTNode>> body;
    while (!isAtEndFn() && peekFn().type != TokenType::RBRACE) {
        body.push_back(parseNodeFn());
    }
    if (peekFn().type == TokenType::RBRACE) advanceFn(); // consume '}'
    return body;
}

// Statement -> VarDecl | Assignment
std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (peek().type == TokenType::LET || peek().type == TokenType::CONST) {
        return parseVarDecl();
    }
    // Could be assignment
    if (peek().type == TokenType::IDENTIFIER) {
        // Look ahead to see if it's an assignment
        if (pos + 1 < tokens.size() && tokens[pos + 1].type == TokenType::EQUALS) {
            return parseAssignment();
        }
    }
    return parseExpression();
}

// VarDecl -> (let|const) IDENTIFIER = Expression ;
std::shared_ptr<ASTNode> Parser::parseVarDecl() {
    advance(); // consume 'let' or 'const'
    std::string name = expect(TokenType::IDENTIFIER, "Expected variable name").value;
    expect(TokenType::EQUALS, "Expected '='");
    auto value = parseExpression();
    expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_shared<VarDeclNode>(name, value);
}

// IfStatement -> if ( Expression ) { Node* } ( else { Node* } )?
std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    advance(); // consume 'if'
    expect(TokenType::LPAREN, "Expected '(' after 'if'");
    auto cond = parseExpression();
    expect(TokenType::RPAREN, "Expected ')' after if condition");

    // then-branch
    expect(TokenType::LBRACE, "Expected '{' after if condition");
    auto ifNode = std::make_shared<IfNode>(cond);
    while (!isAtEnd() && peek().type != TokenType::RBRACE) {
        ifNode->thenBranch.push_back(parseNode());
    }
    expect(TokenType::RBRACE, "Expected '}' to close if block");

    // optional else-branch
    if (!isAtEnd() && peek().type == TokenType::ELSE) {
        advance(); // consume 'else'
        expect(TokenType::LBRACE, "Expected '{' after 'else'");
        while (!isAtEnd() && peek().type != TokenType::RBRACE) {
            ifNode->elseBranch.push_back(parseNode());
        }
        expect(TokenType::RBRACE, "Expected '}' to close else block");
    }

    return ifNode;
}

// ForStatement -> for ( (let|const)? IDENT = Expr ; Expr ; IDENT = Expr ) { Node* }
std::shared_ptr<ASTNode> Parser::parseForStatement() {
    advance(); // consume 'for'
    expect(TokenType::LPAREN, "Expected '(' after 'for'");

    // init: let i = 0;  (the semicolon is consumed inside parseVarDecl)
    std::shared_ptr<ASTNode> init;
    if (peek().type == TokenType::LET || peek().type == TokenType::CONST) {
        init = parseVarDecl(); // consumes 'let i = 0 ;'
    } else {
        init = parseAssignment(); // e.g. i = 0 ;
    }

    // condition: i <= 3 ;
    auto cond = parseExpression();
    expect(TokenType::SEMICOLON, "Expected ';' after for-condition");

    // increment: i = i + 1  (no semicolon — closed by ')')
    std::shared_ptr<ASTNode> inc;
    if (peek().type == TokenType::IDENTIFIER &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1].type == TokenType::EQUALS) {
        std::string name = advance().value; // IDENT
        advance();                          // consume '='
        auto val = parseExpression();
        inc = std::make_shared<AssignmentNode>(name, val);
        // no semicolon expected here — the ')' closes the header
    }

    expect(TokenType::RPAREN, "Expected ')' to close for header");

    // body
    expect(TokenType::LBRACE, "Expected '{' to open for body");
    auto forNode = std::make_shared<ForNode>(init, cond, inc);
    while (!isAtEnd() && peek().type != TokenType::RBRACE) {
        forNode->body.push_back(parseNode());
    }
    expect(TokenType::RBRACE, "Expected '}' to close for body");

    return forNode;
}

// Assignment -> IDENTIFIER = Expression ;
std::shared_ptr<ASTNode> Parser::parseAssignment() {
    std::string name = expect(TokenType::IDENTIFIER, "Expected variable name").value;
    expect(TokenType::EQUALS, "Expected '='");
    auto value = parseExpression();
    expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_shared<AssignmentNode>(name, value);
}

// Expression -> Comparison ( (&&|||) Comparison )*
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

// Comparison -> Term ( (==|!=|<|>|<=|>=) Term )*
std::shared_ptr<ASTNode> Parser::parseComparison() {
    auto left = parseTerm();
    
    // NOTE: TAG_OPEN (<) and TAG_CLOSE (>) are intentionally NOT used here —
    // they denote HTML tags, not comparison operators.
    // Use dedicated LESS_THAN / GREATER_THAN tokens for JS comparisons.
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

// Term -> Factor ( (+|-) Factor )*
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

// Factor -> Primary ( (*|/) Primary )*
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

// Primary -> NUMBER | STRING | IDENTIFIER | ( Expression )
std::shared_ptr<ASTNode> Parser::parsePrimary() {
    // Number literal
    if (peek().type == TokenType::NUMBER) {
        double value = std::stod(advance().value);
        return std::make_shared<NumberNode>(value);
    }
    
    // String literal
    if (peek().type == TokenType::STRING) {
        std::string value = advance().value;
        return std::make_shared<StringNode>(value);
    }
    
    // Identifier (variable reference)
    if (peek().type == TokenType::IDENTIFIER) {
        std::string name = advance().value;
        return std::make_shared<IdentifierNode>(name);
    }
    
    // Parenthesized expression
    if (peek().type == TokenType::LPAREN) {
        advance(); // consume '('
        auto expr = parseExpression();
        expect(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    throw std::runtime_error("Unexpected token in expression at line " + 
                            std::to_string(peek().line));
}
