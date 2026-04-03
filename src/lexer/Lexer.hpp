#pragma once
#include <string>
#include <vector>

enum class TokenType {
    TAG_OPEN,
    TAG_CLOSE,
    SLASH,
    EQUALS,
    
    LET,
    CONST,
    FUNCTION,
    IF,
    ELSE,
    RETURN,
    FOR,
    WHILE,
    
    IDENTIFIER,
    STRING,
    NUMBER,
    
    PLUS,
    MINUS,
    ASTERISK,
    SEMICOLON,
    COMMA,
    DOT,
    COLON,
    
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    
    DOUBLE_EQUALS,
    NOT_EQUALS,
    LESS_EQUALS,
    GREATER_EQUALS,
    LESS_THAN,
    GREATER_THAN,
    AND,
    OR,
    NOT,
    
    EOF_TOKEN,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string src;
    size_t pos = 0;
    int line = 1;
    int column = 1;

    char peek(int offset = 0);
    char advance();
    void skipWhitespace();
    void skipComment();
    Token readIdentifier();
    Token readNumber();
    Token readString();
    bool isKeyword(const std::string& word, TokenType& type);
};
