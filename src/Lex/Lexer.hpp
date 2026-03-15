#pragma once
#include <string>
#include <vector>

enum class TokenType {
    // HTML Tokens
    TAG_OPEN,    // <
    TAG_CLOSE,   // >
    SLASH,       // /
    EQUALS,      // =
    
    // JS Keywords
    LET,         // let
    CONST,       // const
    FUNCTION,    // function
    IF,          // if
    ELSE,        // else
    RETURN,      // return
    FOR,         // for
    WHILE,       // while
    
    // Identifiers and Literals
    IDENTIFIER,  // div, x, myBtn
    STRING,      // "hello"
    NUMBER,      // 10, 5.5
    
    // Single-character operators
    PLUS,        // +
    MINUS,       // -
    ASTERISK,    // *
    SEMICOLON,   // ;
    COMMA,       // ,
    DOT,         // .
    COLON,       // :
    
    // Parentheses, Braces, Brackets
    LPAREN,      // (
    RPAREN,      // )
    LBRACE,      // {
    RBRACE,      // }
    LBRACKET,    // [
    RBRACKET,    // ]
    
    // Multi-character operators
    DOUBLE_EQUALS,  // ==
    NOT_EQUALS,     // !=
    LESS_EQUALS,    // <=
    GREATER_EQUALS, // >=
    LESS_THAN,      // <
    GREATER_THAN,   // >
    AND,            // &&
    OR,             // ||
    NOT,            // !
    
    // System
    EOF_TOKEN,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;  // Added column tracking for better error messages
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
