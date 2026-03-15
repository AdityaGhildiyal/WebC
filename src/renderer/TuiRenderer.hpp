#pragma once
#include "../html/HtmlNode.hpp"
#include <string>
#include <vector>
#include <memory>

class TuiRenderer {
public:
    TuiRenderer();
    void render(const std::vector<std::shared_ptr<HtmlNode>>& roots);

private:
    // Terminal width for layout
    int termWidth = 80;

    // ANSI helpers
    std::string color(const char* code) { return std::string(code); }
    void clearScreen();
    void printLine(const std::string& s = "");
    void printHRule(char ch = '-', int width = -1, const char* col = nullptr);

    // Element renderers
    void renderNode(const std::shared_ptr<HtmlNode>& node, int depth, bool insideList = false);
    void renderChildren(const std::shared_ptr<HtmlNode>& node, int depth, bool insideList = false);

    // Block elements
    void renderDiv(const std::shared_ptr<HtmlNode>& node, int depth);
    void renderHeading(const std::shared_ptr<HtmlNode>& node, int level);
    void renderParagraph(const std::shared_ptr<HtmlNode>& node);
    void renderUnorderedList(const std::shared_ptr<HtmlNode>& node, int depth);
    void renderOrderedList(const std::shared_ptr<HtmlNode>& node, int depth);
    void renderListItem(const std::shared_ptr<HtmlNode>& node, int depth, const std::string& bullet);
    void renderTable(const std::shared_ptr<HtmlNode>& node);
    void renderTableRow(const std::shared_ptr<HtmlNode>& row, bool isHeader, int colWidth);
    void renderForm(const std::shared_ptr<HtmlNode>& node, int depth);
    void renderInput(const std::shared_ptr<HtmlNode>& node);
    void renderButton(const std::shared_ptr<HtmlNode>& node);
    void renderImage(const std::shared_ptr<HtmlNode>& node);
    void renderLink(const std::shared_ptr<HtmlNode>& node);
    void renderCode(const std::shared_ptr<HtmlNode>& node);
    void renderPre(const std::shared_ptr<HtmlNode>& node);
    void renderBlockquote(const std::shared_ptr<HtmlNode>& node, int depth);
    void renderHr();
    void renderNav(const std::shared_ptr<HtmlNode>& node, int depth);
    void renderHeader(const std::shared_ptr<HtmlNode>& node, int depth);
    void renderFooter(const std::shared_ptr<HtmlNode>& node, int depth);
    void renderSection(const std::shared_ptr<HtmlNode>& node, int depth);

    // Inline text with inline elements resolved
    std::string resolveInlineText(const std::shared_ptr<HtmlNode>& node);
    std::string getNodeText(const std::shared_ptr<HtmlNode>& node);

    // Box drawing
    void drawBox(const std::string& title, const std::string& content,
                 const char* borderColor, const char* textColor, int indent = 0);

    // Word-wrap a string to fit within width
    std::vector<std::string> wordWrap(const std::string& text, int width) const;
    
    // Repeat a char N times
    std::string repeat(const std::string& s, int n) const;
};
