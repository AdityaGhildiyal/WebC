#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <iostream>

enum class SymbolType { 
    NUMBER, 
    STRING, 
    HTML_ELEMENT,
    FUNCTION,
    UNKNOWN
};

struct Symbol {
    std::string name;
    SymbolType type;
    int line;
    bool isConst = false;  
};

class SymbolTable {
public:
    SymbolTable() {
        scopes.push_back({});
    }

    void pushScope() {
        scopes.push_back({});
        std::cout << "[SymbolTable] Pushed new scope (depth: " << scopes.size() << ")\n";
    }

    void popScope() {
        if (scopes.size() > 1) {
            scopes.pop_back();
            std::cout << "[SymbolTable] Popped scope (depth: " << scopes.size() << ")\n";
        }
    }

    void define(const std::string& name, SymbolType type, int line, bool isConst = false) {
        if (scopes.back().count(name)) {
            throw std::runtime_error("Semantic Error: Symbol '" + name + 
                                   "' already defined at line " + std::to_string(line));
        }
        scopes.back()[name] = {name, type, line, isConst};
        std::cout << "[SymbolTable] Defined: " << name << " (type: " << typeToString(type) 
                  << ", line: " << line << ")\n";
    }

    Symbol lookup(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(name)) {
                return (*it)[name];
            }
        }
        throw std::runtime_error("Semantic Error: Undefined symbol '" + name + "'");
    }

    bool exists(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(name)) return true;
        }
        return false;
    }

    size_t getScopeDepth() const {
        return scopes.size();
    }

    void printAllSymbols() {
        std::cout << "\n[SymbolTable] All Symbols:\n";
        for (size_t i = 0; i < scopes.size(); i++) {
            std::cout << "  Scope " << i << ":\n";
            for (const auto& pair : scopes[i]) {
                std::cout << "    - " << pair.first << " (" << typeToString(pair.second.type) << ")\n";
            }
        }
        std::cout << "\n";
    }

private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;

    std::string typeToString(SymbolType type) {
        switch (type) {
            case SymbolType::NUMBER: return "NUMBER";
            case SymbolType::STRING: return "STRING";
            case SymbolType::HTML_ELEMENT: return "HTML_ELEMENT";
            case SymbolType::FUNCTION: return "FUNCTION";
            case SymbolType::UNKNOWN: return "UNKNOWN";
            default: return "UNDEFINED";
        }
    }
};
