#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdio>

// HTML Element Node — stores tag, attributes, text, and children
class HtmlNode {
public:
    std::string tag;                          // "div", "h1", "p", etc.
    std::map<std::string, std::string> attrs; // All attributes: id, class, href, style...
    std::string text;                         // Direct text content
    std::vector<std::shared_ptr<HtmlNode>> children;

    HtmlNode() = default;
    HtmlNode(const std::string& t) : tag(t) {}

    std::string attr(const std::string& key, const std::string& def = "") const {
        auto it = attrs.find(key);
        return it != attrs.end() ? it->second : def;
    }

    void debugPrint(int indent = 0) const {
        for (int i = 0; i < indent; i++) printf("  ");
        printf("<%s", tag.c_str());
        for (auto& pair : attrs)
            printf(" %s=\"%s\"", pair.first.c_str(), pair.second.c_str());
        printf(">");
        if (!text.empty()) printf(" \"%s\"", text.c_str());
        printf("\n");
        for (auto& child : children) child->debugPrint(indent + 1);
    }
};
