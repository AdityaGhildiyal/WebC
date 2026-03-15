#include "HtmlNodeGen.hpp"
#include <sstream>
#include <cmath>
#include <iostream>

// ─── Helpers ───────────────────────────────────────────────────────────────

std::string HtmlNodeGen::fmtNum(double v) const {
    // Print integers without decimal point: 42 not 42.000000
    if (v == std::floor(v) && std::abs(v) < 1e15) {
        std::ostringstream ss;
        ss << static_cast<long long>(v);
        return ss.str();
    }
    // For floats, up to 4 significant decimal places, strip trailing zeros
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

// ─── Expression Evaluators ─────────────────────────────────────────────────

double HtmlNodeGen::evalNum(std::shared_ptr<ASTNode> node) {
    if (!node) return 0.0;

    if (auto n = std::dynamic_pointer_cast<NumberNode>(node))
        return n->value;

    if (auto id = std::dynamic_pointer_cast<IdentifierNode>(node)) {
        auto it = numVars.find(id->name);
        return it != numVars.end() ? it->second : 0.0;
    }

    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        double L = evalNum(bin->left);
        double R = evalNum(bin->right);
        switch (bin->op) {
            case '+': return L + R;
            case '-': return L - R;
            case '*': return L * R;
            case '/': return (R != 0.0) ? L / R : 0.0;
            case '%': return std::fmod(L, R);
            default:  return 0.0;
        }
    }

    if (auto str = std::dynamic_pointer_cast<StringNode>(node)) {
        // Try to convert string to number
        try { return std::stod(str->value); } catch (...) { return 0.0; }
    }

    return 0.0;
}

std::string HtmlNodeGen::evalStr(std::shared_ptr<ASTNode> node) {
    if (!node) return "";

    if (auto str = std::dynamic_pointer_cast<StringNode>(node))
        return str->value;

    if (auto num = std::dynamic_pointer_cast<NumberNode>(node))
        return fmtNum(num->value);

    if (auto id = std::dynamic_pointer_cast<IdentifierNode>(node)) {
        // Try string variable first, then numeric
        auto sit = strVars.find(id->name);
        if (sit != strVars.end()) return sit->second;
        auto nit = numVars.find(id->name);
        if (nit != numVars.end()) return fmtNum(nit->second);
        return id->name; // unresolved identifier — show the name itself
    }

    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        // Numeric binary op → format as string
        return fmtNum(evalNum(node));
    }

    return "";
}

// ─── Statement Executor ────────────────────────────────────────────────────

void HtmlNodeGen::execStatement(std::shared_ptr<ASTNode> node) {
    // let / const x = expr;
    if (auto decl = std::dynamic_pointer_cast<VarDeclNode>(node)) {
        if (!decl->initValue) return;

        // Decide storage type based on the initialiser
        if (std::dynamic_pointer_cast<StringNode>(decl->initValue)) {
            strVars[decl->varName] = evalStr(decl->initValue);
        } else {
            // For string variables being assigned a non-string expression
            // that resolves via string lookup, keep as string
            auto it = strVars.find(decl->varName);
            if (it != strVars.end()) {
                strVars[decl->varName] = evalStr(decl->initValue);
            } else {
                numVars[decl->varName] = evalNum(decl->initValue);
            }
        }
        return;
    }

    // x = expr;  (re-assignment)
    if (auto assign = std::dynamic_pointer_cast<AssignmentNode>(node)) {
        // Update whichever map already holds the variable
        if (strVars.count(assign->varName)) {
            strVars[assign->varName] = evalStr(assign->value);
        } else {
            numVars[assign->varName] = evalNum(assign->value);
        }
        return;
    }
}

// ─── Tag Visitor ───────────────────────────────────────────────────────────

std::shared_ptr<HtmlNode> HtmlNodeGen::visitTag(std::shared_ptr<TagNode> tag) {
    auto node = std::make_shared<HtmlNode>(tag->tagName);

    // Carry over the id attribute
    if (!tag->id.empty()) {
        node->attrs["id"] = tag->id;
        domElements[tag->id] = node; // register in DOM for future lookup
    }

    // Process children — a mixture of sub-tags, JS statements, and expressions
    std::string textBuf; // accumulates consecutive inline text/expressions

    auto flushText = [&]() {
        if (!textBuf.empty()) {
            // Trim leading/trailing spaces
            size_t s = textBuf.find_first_not_of(" \t");
            size_t e = textBuf.find_last_not_of(" \t");
            if (s != std::string::npos)
                node->text += (node->text.empty() ? "" : " ")
                            + textBuf.substr(s, e - s + 1);
            textBuf.clear();
        }
    };

    for (auto& child : tag->children) {
        // ── Sub-tag ──────────────────────────────────────────────────────
        if (auto childTag = std::dynamic_pointer_cast<TagNode>(child)) {
            flushText(); // flush any accumulated inline text first
            auto childNode = visitTag(childTag);
            if (childNode) node->children.push_back(childNode);
        }
        // ── JS Statement (let / const / assignment) ───────────────────────
        else if (std::dynamic_pointer_cast<VarDeclNode>(child) ||
                 std::dynamic_pointer_cast<AssignmentNode>(child)) {
            flushText();
            execStatement(child);
        }
        // ── Inline expression → text ──────────────────────────────────────
        else {
            std::string piece = evalStr(child);
            if (!piece.empty()) textBuf += piece;
        }
    }

    flushText();
    return node;
}

// ─── Entry Point ───────────────────────────────────────────────────────────

std::vector<std::shared_ptr<HtmlNode>> HtmlNodeGen::generate(
    const std::vector<std::shared_ptr<ASTNode>>& nodes)
{
    // Reset interpreter state for a fresh compilation
    numVars.clear();
    strVars.clear();
    domElements.clear();

    std::vector<std::shared_ptr<HtmlNode>> roots;

    for (auto& node : nodes) {
        if (auto tag = std::dynamic_pointer_cast<TagNode>(node)) {
            // Top-level HTML element
            auto htmlNode = visitTag(tag);
            if (htmlNode) roots.push_back(htmlNode);
        } else {
            // Top-level JS statement (valid in WebC)
            execStatement(node);
        }
    }

    return roots;
}
