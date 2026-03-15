#include "Lexer.hpp"
#include <cctype>
#include <unordered_map>

Lexer::Lexer(const std::string& source) : src(source) {}

char Lexer::peek(int offset) { 
    size_t index = pos + offset;
    return index < src.size() ? src[index] : '\0'; 
}

char Lexer::advance() {
    char c = src[pos++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (isspace(peek())) {
        advance();
    }
}

void Lexer::skipComment() {
    // Single-line comment: //
    if (peek() == '/' && peek(1) == '/') {
        advance(); // skip first /
        advance(); // skip second /
        while (peek() != '\n' && peek() != '\0') {
            advance();
        }
    }
    // Multi-line comment: /* */
    else if (peek() == '/' && peek(1) == '*') {
        advance(); // skip /
        advance(); // skip *
        while (true) {
            if (peek() == '\0') break; // EOF
            if (peek() == '*' && peek(1) == '/') {
                advance(); // skip *
                advance(); // skip /
                break;
            }
            advance();
        }
    }
}

bool Lexer::isKeyword(const std::string& word, TokenType& type) {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"let", TokenType::LET},
        {"const", TokenType::CONST},
        {"function", TokenType::FUNCTION},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"return", TokenType::RETURN},
        {"for", TokenType::FOR},
        {"while", TokenType::WHILE}
    };
    
    auto it = keywords.find(word);
    if (it != keywords.end()) {
        type = it->second;
        return true;
    }
    return false;
}

Token Lexer::readIdentifier() {
    int startCol = column;
    std::string val;
    while (isalnum(peek()) || peek() == '_') {
        val += advance();
    }
    
    // Check if it's a keyword
    TokenType type;
    if (isKeyword(val, type)) {
        return {type, val, line, startCol};
    }
    
    return {TokenType::IDENTIFIER, val, line, startCol};
}

Token Lexer::readNumber() {
    int startCol = column;
    std::string val;
    bool hasDecimal = false;
    
    while (isdigit(peek()) || peek() == '.') {
        if (peek() == '.') {
            if (hasDecimal) break; // Second decimal point, stop
            hasDecimal = true;
        }
        val += advance();
    }
    
    return {TokenType::NUMBER, val, line, startCol};
}

Token Lexer::readString() {
    int startCol = column;
    advance(); // Skip opening quote
    std::string val;
    
    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\\') {
            advance(); // Skip escape character
            char escaped = peek();
            switch (escaped) {
                case 'n': val += '\n'; break;
                case 't': val += '\t'; break;
                case 'r': val += '\r'; break;
                case '\\': val += '\\'; break;
                case '"': val += '"'; break;
                default: val += escaped; break;
            }
            advance();
        } else {
            val += advance();
        }
    }
    
    advance(); // Skip closing quote
    return {TokenType::STRING, val, line, startCol};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (pos < src.size()) {
        skipWhitespace();
        
        // Check for comments
        if (peek() == '/' && (peek(1) == '/' || peek(1) == '*')) {
            skipComment();
            continue;
        }
        
        char c = peek();
        if (c == '\0') break;
        
        int startCol = column;

        // Identifiers and keywords
        if (isalpha(c) || c == '_') {
            tokens.push_back(readIdentifier());
        }
        // Numbers
        else if (isdigit(c)) {
            tokens.push_back(readNumber());
        }
        // Strings
        else if (c == '"') {
            tokens.push_back(readString());
        }
        // Multi-character operators and single-character tokens
        else {
            char current = advance();
            
            switch (current) {
                // Check for multi-character operators
                case '=':
                    if (peek() == '=') {
                        advance();
                        tokens.push_back({TokenType::DOUBLE_EQUALS, "==", line, startCol});
                    } else {
                        tokens.push_back({TokenType::EQUALS, "=", line, startCol});
                    }
                    break;
                    
                case '!':
                    if (peek() == '=') {
                        advance();
                        tokens.push_back({TokenType::NOT_EQUALS, "!=", line, startCol});
                    } else {
                        tokens.push_back({TokenType::NOT, "!", line, startCol});
                    }
                    break;
                    
                case '<':
                    if (peek() == '=') {
                        advance();
                        tokens.push_back({TokenType::LESS_EQUALS, "<=", line, startCol});
                    } else {
                        // Could be TAG_OPEN or LESS_THAN - context dependent
                        // For now, we'll use TAG_OPEN for HTML compatibility
                        tokens.push_back({TokenType::TAG_OPEN, "<", line, startCol});
                    }
                    break;
                    
                case '>':
                    if (peek() == '=') {
                        advance();
                        tokens.push_back({TokenType::GREATER_EQUALS, ">=", line, startCol});
                    } else {
                        // Could be TAG_CLOSE or GREATER_THAN
                        tokens.push_back({TokenType::TAG_CLOSE, ">", line, startCol});
                    }
                    break;
                    
                case '&':
                    if (peek() == '&') {
                        advance();
                        tokens.push_back({TokenType::AND, "&&", line, startCol});
                    } else {
                        tokens.push_back({TokenType::UNKNOWN, "&", line, startCol});
                    }
                    break;
                    
                case '|':
                    if (peek() == '|') {
                        advance();
                        tokens.push_back({TokenType::OR, "||", line, startCol});
                    } else {
                        tokens.push_back({TokenType::UNKNOWN, "|", line, startCol});
                    }
                    break;
                
                // Single-character tokens
                case '/': tokens.push_back({TokenType::SLASH, "/", line, startCol}); break;
                case '+': tokens.push_back({TokenType::PLUS, "+", line, startCol}); break;
                case '-': tokens.push_back({TokenType::MINUS, "-", line, startCol}); break;
                case '*': tokens.push_back({TokenType::ASTERISK, "*", line, startCol}); break;
                case ';': tokens.push_back({TokenType::SEMICOLON, ";", line, startCol}); break;
                case ',': tokens.push_back({TokenType::COMMA, ",", line, startCol}); break;
                case '.': tokens.push_back({TokenType::DOT, ".", line, startCol}); break;
                case ':': tokens.push_back({TokenType::COLON, ":", line, startCol}); break;
                case '(': tokens.push_back({TokenType::LPAREN, "(", line, startCol}); break;
                case ')': tokens.push_back({TokenType::RPAREN, ")", line, startCol}); break;
                case '{': tokens.push_back({TokenType::LBRACE, "{", line, startCol}); break;
                case '}': tokens.push_back({TokenType::RBRACE, "}", line, startCol}); break;
                case '[': tokens.push_back({TokenType::LBRACKET, "[", line, startCol}); break;
                case ']': tokens.push_back({TokenType::RBRACKET, "]", line, startCol}); break;
                
                default: 
                    tokens.push_back({TokenType::UNKNOWN, std::string(1, current), line, startCol}); 
                    break;
            }
        }
    }
    
    tokens.push_back({TokenType::EOF_TOKEN, "EOF", line, column});
    return tokens;
}
