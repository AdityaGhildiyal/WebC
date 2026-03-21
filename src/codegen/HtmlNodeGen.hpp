#pragma once
#include "../ast/ASTNodes.hpp"
#include "../html/HtmlNode.hpp"
#include <memory>
#include <vector>
#include <map>
#include <string>

// HtmlNodeGen — the WebC compiler's backend.
//
// Instead of generating LLVM IR (which needs a full LLVM toolchain),
// this evaluator walks the validated WebC AST and produces an HtmlNode
// tree that the TuiRenderer can display directly.
//
// Pipeline:
//   .webc source  →  Lexer → Parser → SemanticAnalyzer → HtmlNodeGen
//                                                              ↓
//                                                       HtmlNode tree
//                                                              ↓
//                                                       TuiRenderer  →  Terminal
//
class HtmlNodeGen {
public:
    // Entry point — returns the root nodes to pass to TuiRenderer
    std::vector<std::shared_ptr<HtmlNode>> generate(
        const std::vector<std::shared_ptr<ASTNode>>& nodes);

private:
    // Runtime state — the "heap" for the WebC interpreter
    std::map<std::string, double>      numVars;  // numeric variables
    std::map<std::string, std::string> strVars;  // string variables
    // DOM registry: id → node pointer (for future querySelector support)
    std::map<std::string, std::shared_ptr<HtmlNode>> domElements;

    // ------- Visitors -------
    // Convert a WebC TagNode into an HtmlNode (recursively)
    std::shared_ptr<HtmlNode> visitTag(std::shared_ptr<TagNode> tag);

    // Generate child HtmlNodes from a list of AST nodes into a parent
    void generateInto(const std::vector<std::shared_ptr<ASTNode>>& nodes,
                      std::shared_ptr<HtmlNode> parent);

    // Execute a JS-like statement (let/const declaration, assignment)
    void execStatement(std::shared_ptr<ASTNode> node);

    // Evaluate control-flow nodes, appending resulting HtmlNodes to parent
    void evalIf(std::shared_ptr<IfNode> node, std::shared_ptr<HtmlNode> parent);
    void evalFor(std::shared_ptr<ForNode> node, std::shared_ptr<HtmlNode> parent);

    // ------- Expression evaluators -------
    double      evalNum(std::shared_ptr<ASTNode> node);   // → numeric value
    bool        evalCond(std::shared_ptr<ASTNode> node);  // → boolean condition
    std::string evalStr(std::shared_ptr<ASTNode> node);   // → string representation

    // Helper: format a double cleanly (no trailing .000000)
    std::string fmtNum(double v) const;
};
