#include "HtmlParser.hpp"
#include <cctype>
#include <algorithm>
#include <stdexcept>

HtmlParser::HtmlParser(const std::string& source) : src(source) {}

char HtmlParser::peek(int offset) const {
    size_t i = pos + offset;
    return i < src.size() ? src[i] : '\0';
}

char HtmlParser::advance() {
    return pos < src.size() ? src[pos++] : '\0';
}

bool HtmlParser::isAtEnd() const {
    return pos >= src.size();
}

void HtmlParser::skipWhitespace() {
    while (!isAtEnd() && isspace(peek())) advance();
}

void HtmlParser::skipComment() {
    while (!isAtEnd()) {
        if (peek() == '-' && peek(1) == '-' && peek(2) == '>') {
            advance(); advance(); advance(); 
            return;
        }
        advance();
    }
}

bool HtmlParser::isSelfClosing(const std::string& tag) const {
    static const std::vector<std::string> selfClose = {
        "area","base","br","col","embed","hr","img","input",
        "link","meta","param","source","track","wbr"
    };
    std::string lower = tag;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    for (auto& s : selfClose) if (s == lower) return true;
    return false;
}

std::string HtmlParser::readTagName() {
    std::string name;
    while (!isAtEnd() && (isalnum(peek()) || peek() == '-' || peek() == '_' || peek() == ':'))
        name += advance();
    return name;
}

std::string HtmlParser::readAttrName() {
    std::string name;
    while (!isAtEnd() && peek() != '=' && peek() != '>' && peek() != '/' && !isspace(peek()))
        name += advance();
    return name;
}

std::string HtmlParser::readAttrValue() {
    char quote = peek();
    if (quote == '"' || quote == '\'') {
        advance(); 
        std::string val;
        while (!isAtEnd() && peek() != quote) {
            if (peek() == '&') {
                std::string entity;
                size_t start = pos;
                advance();
                while (!isAtEnd() && peek() != ';' && !isspace(peek()) && pos - start < 10)
                    entity += advance();
                if (peek() == ';') {
                    advance(); 
                    if      (entity == "amp")  val += '&';
                    else if (entity == "lt")   val += '<';
                    else if (entity == "gt")   val += '>';
                    else if (entity == "quot") val += '"';
                    else if (entity == "apos") val += '\'';
                    else if (entity == "nbsp") val += ' ';
                    else val += '&' + entity + ';';
                } else {
                    val += '&' + entity;
                }
            } else {
                val += advance();
            }
        }
        if (!isAtEnd()) advance(); 
        return val;
    }
    std::string val;
    while (!isAtEnd() && !isspace(peek()) && peek() != '>' && peek() != '/')
        val += advance();
    return val;
}

void HtmlParser::parseAttributes(std::shared_ptr<HtmlNode>& node) {
    while (!isAtEnd()) {
        skipWhitespace();
        if (peek() == '>' || peek() == '/' || isAtEnd()) break;

        std::string name = readAttrName();
        if (name.empty()) { advance(); continue; } 

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        skipWhitespace();
        if (peek() == '=') {
            advance(); 
            skipWhitespace();
            node->attrs[name] = readAttrValue();
        } else {
            node->attrs[name] = "true";
        }
    }
}

std::string HtmlParser::readText() {
    std::string text;
    while (!isAtEnd() && peek() != '<') {
        if (peek() == '&') {
            std::string entity;
            size_t start = pos;
            advance();
            while (!isAtEnd() && peek() != ';' && !isspace(peek()) && pos - start < 10)
                entity += advance();
            if (!isAtEnd() && peek() == ';') {
                advance();
                if      (entity == "amp")  text += '&';
                else if (entity == "lt")   text += '<';
                else if (entity == "gt")   text += '>';
                else if (entity == "quot") text += '"';
                else if (entity == "nbsp") text += ' ';
                else text += '&' + entity + ';';
            } else {
                text += '&' + entity;
            }
        } else {
            text += advance();
        }
    }
    size_t start = text.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = text.find_last_not_of(" \t\n\r");
    return text.substr(start, end - start + 1);
}

std::shared_ptr<HtmlNode> HtmlParser::parseElement() {
    
    if (peek() == '!' && peek(1) == '-' && peek(2) == '-') {
        advance(); advance(); advance(); 
        skipComment();
        return nullptr;
    }

    if (peek() == '!') {
        while (!isAtEnd() && peek() != '>') advance();
        if (!isAtEnd()) advance(); 
        return nullptr;
    }

    if (peek() == '/') {
        while (!isAtEnd() && peek() != '>') advance();
        if (!isAtEnd()) advance(); 
        return nullptr;
    }

    std::string tagName = readTagName();
    if (tagName.empty()) {
        return nullptr;
    }
    std::transform(tagName.begin(), tagName.end(), tagName.begin(), ::tolower);

    auto node = std::make_shared<HtmlNode>(tagName);
    parseAttributes(node);

    if (peek() == '/') {
        advance(); 
        if (peek() == '>') advance(); 
        return node;
    }

    if (peek() == '>') advance();

    if (isSelfClosing(tagName)) return node;

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;

        if (peek() == '<') {
            advance();

            if (peek() == '/') {
                advance();
                std::string closeName = readTagName();
                std::transform(closeName.begin(), closeName.end(), closeName.begin(), ::tolower);
                while (!isAtEnd() && peek() != '>') advance();
                if (!isAtEnd()) advance(); 

                if (closeName == tagName || closeName.empty()) {
                    break; 
                }
                break;
            }

            auto child = parseElement();
            if (child) node->children.push_back(child);
        } else {

            std::string text = readText();
            if (!text.empty()) {
                auto textNode = std::make_shared<HtmlNode>("#text");
                textNode->text = text;
                node->children.push_back(textNode);
            }
        }
    }

    bool allText = !node->children.empty();
    for (auto& ch : node->children)
        if (ch->tag != "#text") { allText = false; break; }

    if (allText) {
        std::string combined;
        for (auto& ch : node->children) {
            if (!combined.empty() && !ch->text.empty()) combined += " ";
            combined += ch->text;
        }
        node->text = combined;
        node->children.clear();
    }

    return node;
}

std::shared_ptr<HtmlNode> HtmlParser::parseNode() {
    skipWhitespace();
    if (isAtEnd()) return nullptr;

    if (peek() == '<') {
        advance(); 
        return parseElement();
    }

    std::string text = readText();
    if (!text.empty()) {
        auto node = std::make_shared<HtmlNode>("#text");
        node->text = text;
        return node;
    }
    return nullptr;
}

std::vector<std::shared_ptr<HtmlNode>> HtmlParser::parse() {
    std::vector<std::shared_ptr<HtmlNode>> roots;
    while (!isAtEnd()) {
        auto node = parseNode();
        if (node) roots.push_back(node);
    }
    return roots;
}
