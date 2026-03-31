#pragma once
#include "HtmlNode.hpp"
#include <string>
#include <vector>
#include <memory>

class HtmlParser {
public:
    explicit HtmlParser(const std::string& source);
    
    std::vector<std::shared_ptr<HtmlNode>> parse();

private:
    std::string src;
    size_t pos = 0;

    char peek(int offset = 0) const;
    char advance();
    void skipWhitespace();
    void skipComment();          
    bool isAtEnd() const;

    std::string readTagName();
    std::string readAttrName();
    std::string readAttrValue(); 
    std::string readText();      
    void        parseAttributes(std::shared_ptr<HtmlNode>& node);

    bool isSelfClosing(const std::string& tag) const;

    std::shared_ptr<HtmlNode> parseNode();
    std::shared_ptr<HtmlNode> parseElement();
};
