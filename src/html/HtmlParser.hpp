#pragma once
#include "HtmlNode.hpp"
#include <string>
#include <vector>
#include <memory>

class HtmlParser {
public:
    explicit HtmlParser(const std::string& source);
    
    // Parse the full document, return root nodes (usually <html> or body children)
    std::vector<std::shared_ptr<HtmlNode>> parse();

private:
    std::string src;
    size_t pos = 0;

    // Low-level helpers
    char peek(int offset = 0) const;
    char advance();
    void skipWhitespace();
    void skipComment();           // <!-- ... -->
    bool isAtEnd() const;

    // Parsing helpers
    std::string readTagName();
    std::string readAttrName();
    std::string readAttrValue();  // handles "value" or 'value' or bare-word
    std::string readText();       // read raw text until '<'
    void        parseAttributes(std::shared_ptr<HtmlNode>& node);

    // Self-closing tags that have no children / closing tag
    bool isSelfClosing(const std::string& tag) const;

    // Main recursive parser
    std::shared_ptr<HtmlNode> parseNode();
    std::shared_ptr<HtmlNode> parseElement();
};
