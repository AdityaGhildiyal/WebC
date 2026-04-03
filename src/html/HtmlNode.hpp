#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

class HtmlNode {
public:
    std::string tag;                          
    std::map<std::string, std::string> attrs;
    std::string text;                         
    std::vector<std::shared_ptr<HtmlNode>> children;

    HtmlNode() = default;
    HtmlNode(const std::string& t) : tag(t) {}

    std::string attr(const std::string& key, const std::string& def = "") const {
        auto it = attrs.find(key);
        return it != attrs.end() ? it->second : def;
    }
};
