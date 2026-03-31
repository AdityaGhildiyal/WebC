#pragma once
#include "../ast/ASTNodes.hpp"
#include "../html/HtmlNode.hpp"
#include <memory>
#include <vector>
#include <map>
#include <string>


class HtmlNodeGen {
public:
    std::vector<std::shared_ptr<HtmlNode>> generate(
        const std::vector<std::shared_ptr<ASTNode>>& nodes);

private:
    std::map<std::string, double>      numVars;  
    std::map<std::string, std::string> strVars;  
    std::map<std::string, std::shared_ptr<HtmlNode>> domElements;


    std::shared_ptr<HtmlNode> visitTag(std::shared_ptr<TagNode> tag);

    void generateInto(const std::vector<std::shared_ptr<ASTNode>>& nodes,
                      std::shared_ptr<HtmlNode> parent);

    void execStatement(std::shared_ptr<ASTNode> node);

    void evalIf(std::shared_ptr<IfNode> node, std::shared_ptr<HtmlNode> parent);
    void evalFor(std::shared_ptr<ForNode> node, std::shared_ptr<HtmlNode> parent);
    void evalWhile(std::shared_ptr<WhileNode> node, std::shared_ptr<HtmlNode> parent);

    double      evalNum(std::shared_ptr<ASTNode> node); 
    bool        evalCond(std::shared_ptr<ASTNode> node);  
    std::string evalStr(std::shared_ptr<ASTNode> node);   

    std::string fmtNum(double v) const;
};
