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

// evalCond — evaluate a comparison/logical expression as a boolean (0/1)
bool HtmlNodeGen::evalCond(std::shared_ptr<ASTNode> node) {
    if (!node) return false;

    if (auto bin = std::dynamic_pointer_cast<BinaryOpNode>(node)) {
        // Logical operators — recurse
        if (bin->op == '&') return evalCond(bin->left) && evalCond(bin->right);
        if (bin->op == '|') return evalCond(bin->left) || evalCond(bin->right);

        // Comparison operators
        double L = evalNum(bin->left);
        double R = evalNum(bin->right);
        switch (bin->op) {
            case '=': return L == R;          // '==' mapped to '='
            case '!': return L != R;          // '!=' mapped to '!'
            case '<': return L <  R;
            case '>': return L >  R;
            case 'L': return L <= R;          // '<=' mapped to 'L'
            case 'G': return L >= R;          // '>=' mapped to 'G'
            default:  return evalNum(node) != 0.0;
        }
    }

    // Plain number / variable — truthy if non-zero
    return evalNum(node) != 0.0;
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

// evalIf — evaluate an if/else node, appending resulting HtmlNodes to parent
void HtmlNodeGen::evalIf(std::shared_ptr<IfNode> node,
                          std::shared_ptr<HtmlNode> parent) {
    bool result = evalCond(node->condition);
    const auto& branch = result ? node->thenBranch : node->elseBranch;
    generateInto(branch, parent);
}

// evalFor — execute a for-loop at compile time, appending each iteration's
// HtmlNodes to the parent.
void HtmlNodeGen::evalFor(std::shared_ptr<ForNode> node,
                           std::shared_ptr<HtmlNode> parent) {
    // Execute init statement (e.g., let i = 1)
    if (node->init) execStatement(node->init);

    // Safety cap: never run more than 10 000 iterations
    int guard = 0;
    while (guard++ < 10000) {
        // Evaluate condition
        if (!node->condition || !evalCond(node->condition)) break;

        // Generate body nodes for this iteration
        generateInto(node->body, parent);

        // Execute increment (e.g., i = i + 1)
        if (node->increment) execStatement(node->increment);
    }
}

// generateInto — walk a list of AST nodes and push resulting HtmlNode
// children into 'parent'. Used by visitTag() and control-flow evaluators.
void HtmlNodeGen::generateInto(const std::vector<std::shared_ptr<ASTNode>>& nodes,
                                std::shared_ptr<HtmlNode> parent) {
    std::string textBuf;

    auto flushText = [&]() {
        if (!textBuf.empty()) {
            size_t s = textBuf.find_first_not_of(" \t");
            size_t e = textBuf.find_last_not_of(" \t");
            if (s != std::string::npos)
                parent->text += (parent->text.empty() ? "" : " ")
                              + textBuf.substr(s, e - s + 1);
            textBuf.clear();
        }
    };

    for (auto& child : nodes) {
        // Sub-tag
        if (auto childTag = std::dynamic_pointer_cast<TagNode>(child)) {
            flushText();
            auto childNode = visitTag(childTag);
            if (childNode) parent->children.push_back(childNode);
        }
        // if statement
        else if (auto ifNode = std::dynamic_pointer_cast<IfNode>(child)) {
            flushText();
            evalIf(ifNode, parent);
        }
        // for loop
        else if (auto forNode = std::dynamic_pointer_cast<ForNode>(child)) {
            flushText();
            evalFor(forNode, parent);
        }
        // JS statement
        else if (std::dynamic_pointer_cast<VarDeclNode>(child) ||
                 std::dynamic_pointer_cast<AssignmentNode>(child)) {
            flushText();
            execStatement(child);
        }
        // Inline expression → text
        else {
            std::string piece = evalStr(child);
            if (!piece.empty()) textBuf += piece;
        }
    }
    flushText();
}

// ─── Tag Visitor ───────────────────────────────────────────────────────────

std::shared_ptr<HtmlNode> HtmlNodeGen::visitTag(std::shared_ptr<TagNode> tag) {
    auto node = std::make_shared<HtmlNode>(tag->tagName);

    // Carry over the id attribute
    if (!tag->id.empty()) {
        node->attrs["id"] = tag->id;
        domElements[tag->id] = node;
    }

    // Delegate all child processing to generateInto so that
    // if/for nodes inside tags are handled uniformly.
    generateInto(tag->children, node);

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

    // Use a dummy root wrapper so generateInto can append top-level nodes
    auto wrapper = std::make_shared<HtmlNode>("__root__");

    for (auto& node : nodes) {
        if (auto tag = std::dynamic_pointer_cast<TagNode>(node)) {
            auto htmlNode = visitTag(tag);
            if (htmlNode) roots.push_back(htmlNode);
        } else if (auto ifNode = std::dynamic_pointer_cast<IfNode>(node)) {
            evalIf(ifNode, wrapper);
            for (auto& c : wrapper->children) roots.push_back(c);
            wrapper->children.clear();
        } else if (auto forNode = std::dynamic_pointer_cast<ForNode>(node)) {
            evalFor(forNode, wrapper);
            for (auto& c : wrapper->children) roots.push_back(c);
            wrapper->children.clear();
        } else {
            // Top-level JS statement
            execStatement(node);
        }
    }

    return roots;
}
