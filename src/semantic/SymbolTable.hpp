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
    bool isConst = false;  // Track if variable is const
};

class SymbolTable {
public:
    SymbolTable() {
        // Start with global scope
        scopes.push_back({});
    }

    // Push a new scope (e.g., inside a function or block)
    void pushScope() {
        scopes.push_back({});
        std::cout << "[SymbolTable] Pushed new scope (depth: " << scopes.size() << ")\n";
    }

    // Pop the current scope
    void popScope() {
        if (scopes.size() > 1) {
            scopes.pop_back();
            std::cout << "[SymbolTable] Popped scope (depth: " << scopes.size() << ")\n";
        }
    }

    // Define a variable in the current scope
    void define(const std::string& name, SymbolType type, int line, bool isConst = false) {
        if (scopes.back().count(name)) {
            throw std::runtime_error("Semantic Error: Symbol '" + name + 
                                   "' already defined at line " + std::to_string(line));
        }
        scopes.back()[name] = {name, type, line, isConst};
        std::cout << "[SymbolTable] Defined: " << name << " (type: " << typeToString(type) 
                  << ", line: " << line << ")\n";
    }

    // Look up a variable (searching from inner scope to outer)
    Symbol lookup(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(name)) {
                return (*it)[name];
            }
        }
        throw std::runtime_error("Semantic Error: Undefined symbol '" + name + "'");
    }

    // Check if a symbol exists (without throwing)
    bool exists(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (it->count(name)) return true;
        }
        return false;
    }

    // Get current scope depth
    size_t getScopeDepth() const {
        return scopes.size();
    }

    // Print all symbols in all scopes (for debugging)
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
