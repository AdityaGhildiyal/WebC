#include "TuiRenderer.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <cstring>

// ─── ANSI Color Codes ──────────────────────────────────────────────────────
#define RST  "\033[0m"
#define BOLD "\033[1m"
#define DIM  "\033[2m"
#define ITAL "\033[3m"
#define UNDR "\033[4m"

#define FG_BLACK   "\033[30m"
#define FG_RED     "\033[31m"
#define FG_GREEN   "\033[32m"
#define FG_YELLOW  "\033[33m"
#define FG_BLUE    "\033[34m"
#define FG_MAGENTA "\033[35m"
#define FG_CYAN    "\033[36m"
#define FG_WHITE   "\033[37m"
#define FG_BRIGHT_BLACK   "\033[90m"
#define FG_BRIGHT_RED     "\033[91m"
#define FG_BRIGHT_GREEN   "\033[92m"
#define FG_BRIGHT_YELLOW  "\033[93m"
#define FG_BRIGHT_BLUE    "\033[94m"
#define FG_BRIGHT_MAGENTA "\033[95m"
#define FG_BRIGHT_CYAN    "\033[96m"
#define FG_BRIGHT_WHITE   "\033[97m"

#define BG_BLUE    "\033[44m"
#define BG_CYAN    "\033[46m"
#define BG_BLACK   "\033[40m"
#define BG_YELLOW  "\033[43m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_MAGENTA "\033[45m"
#define BG_BRIGHT_BLUE "\033[104m"

// Box-drawing
static const std::string TL = "┌";
static const std::string TR = "┐";
static const std::string BL = "└";
static const std::string BR = "┘";
static const std::string HZ = "─";
static const std::string VT = "│";
static const std::string TM = "┬";
static const std::string BM = "┴";
static const std::string ML = "├";
static const std::string MR = "┤";
static const std::string XX = "┼";

// ─── Constructor ──────────────────────────────────────────────────────────
TuiRenderer::TuiRenderer() {
    // Try to detect terminal width (default 80)
    termWidth = 80;
#ifdef _WIN32
    // Windows: try COLUMNS env or default
    const char* w = std::getenv("COLUMNS");
    if (w) termWidth = std::max(40, std::atoi(w));
#else
    const char* w = std::getenv("COLUMNS");
    if (w) termWidth = std::max(40, std::atoi(w));
#endif
}

// ─── Low-level helpers ─────────────────────────────────────────────────────
void TuiRenderer::clearScreen() {
    std::cout << "\033[2J\033[H";
}

void TuiRenderer::printLine(const std::string& s) {
    std::cout << s << "\n";
}

void TuiRenderer::printHRule(char ch, int width, const char* col) {
    if (width < 0) width = termWidth;
    if (col) std::cout << col;
    for (int i = 0; i < width; i++) std::cout << ch;
    if (col) std::cout << RST;
    std::cout << "\n";
}

std::string TuiRenderer::repeat(const std::string& s, int n) const {
    std::string out;
    for (int i = 0; i < n; i++) out += s;
    return out;
}

std::vector<std::string> TuiRenderer::wordWrap(const std::string& text, int width) const {
    std::vector<std::string> lines;
    if (width <= 0) { lines.push_back(text); return lines; }

    std::istringstream iss(text);
    std::string word, line;
    while (iss >> word) {
        if (!line.empty() && (int)(line.size() + 1 + word.size()) > width) {
            lines.push_back(line);
            line = word;
        } else {
            if (!line.empty()) line += ' ';
            line += word;
        }
    }
    if (!line.empty()) lines.push_back(line);
    if (lines.empty()) lines.push_back("");
    return lines;
}

// ─── Text extraction ───────────────────────────────────────────────────────
std::string TuiRenderer::getNodeText(const std::shared_ptr<HtmlNode>& node) {
    if (!node) return "";
    std::string out = node->text;
    for (auto& ch : node->children) {
        std::string ct = getNodeText(ch);
        if (!ct.empty()) {
            if (!out.empty()) out += " ";
            out += ct;
        }
    }
    return out;
}

std::string TuiRenderer::resolveInlineText(const std::shared_ptr<HtmlNode>& node) {
    if (!node) return "";
    std::string out = node->text;

    for (auto& ch : node->children) {
        std::string ct;
        // Handle inline formatting tags
        if (ch->tag == "strong" || ch->tag == "b") {
            ct = std::string(BOLD) + getNodeText(ch) + RST;
        } else if (ch->tag == "em" || ch->tag == "i") {
            ct = std::string(ITAL) + getNodeText(ch) + RST;
        } else if (ch->tag == "u") {
            ct = std::string(UNDR) + getNodeText(ch) + RST;
        } else if (ch->tag == "code") {
            ct = std::string(BG_BLACK) + FG_BRIGHT_GREEN + " " + getNodeText(ch) + " " + RST;
        } else if (ch->tag == "a") {
            std::string href = ch->attr("href", "#");
            std::string txt = getNodeText(ch);
            if (txt.empty()) txt = href;
            ct = std::string(UNDR) + FG_BRIGHT_BLUE + txt + RST
               + std::string(DIM) + " [" + href + "]" + RST;
        } else if (ch->tag == "span") {
            ct = resolveInlineText(ch);
        } else if (ch->tag == "br") {
            ct = "\n";
        } else {
            ct = resolveInlineText(ch);
        }
        if (!out.empty() && !ct.empty()) out += " ";
        out += ct;
    }
    return out;
}

// ─── Box drawing ───────────────────────────────────────────────────────────
void TuiRenderer::drawBox(const std::string& title, const std::string& content,
                           const char* borderColor, const char* textColor, int indent) {
    int innerW = termWidth - 2 - indent * 2;
    if (innerW < 10) innerW = 10;

    std::string pad(indent * 2, ' ');
    std::string bc = borderColor ? borderColor : "";
    std::string tc = textColor   ? textColor   : "";

    // Top border
    std::cout << pad << bc << TL;
    if (!title.empty()) {
        std::string header = " " + title + " ";
        std::cout << HZ << header;
        int rem = innerW - 2 - (int)title.size() - 2;
        for (int i = 0; i < rem; i++) std::cout << HZ;
    } else {
        for (int i = 0; i < innerW; i++) std::cout << HZ;
    }
    std::cout << TR << RST << "\n";

    // Content lines
    auto lines = wordWrap(content, innerW - 2);
    for (auto& ln : lines) {
        std::cout << pad << bc << VT << RST << " "
                  << tc << ln << RST;
        int pad2 = innerW - 2 - (int)ln.size();
        for (int i = 0; i < pad2; i++) std::cout << ' ';
        std::cout << bc << VT << RST << "\n";
    }

    // Bottom border
    std::cout << pad << bc << BL;
    for (int i = 0; i < innerW; i++) std::cout << HZ;
    std::cout << BR << RST << "\n";
}

// ─── Renderers ─────────────────────────────────────────────────────────────

void TuiRenderer::renderHeading(const std::shared_ptr<HtmlNode>& node, int level) {
    std::string text = resolveInlineText(node);
    if (text.empty()) text = getNodeText(node);

    std::cout << "\n";
    switch (level) {
        case 1:  // H1 — full-width bold cyan bar
        {
            std::string bar = " ═ " + text + " ═";
            int pad = (termWidth - (int)text.size() - 6);
            if (pad < 0) pad = 0;
            std::cout << BOLD << BG_BRIGHT_BLUE << FG_BRIGHT_WHITE;
            std::cout << " ";
            std::cout << text;
            for (int i = 0; i < pad + 4; i++) std::cout << ' ';
            std::cout << RST << "\n\n";
            break;
        }
        case 2:
        {
            std::cout << BOLD << FG_BRIGHT_CYAN << "  " << text << RST
                      << "\n" << FG_CYAN;
            for (int i = 0; i < (int)text.size() + 2; i++) std::cout << "─";
            std::cout << RST << "\n";
            break;
        }
        case 3:
            std::cout << BOLD << FG_BRIGHT_YELLOW << "  ▶ " << text << RST << "\n";
            break;
        case 4:
            std::cout << BOLD << FG_BRIGHT_GREEN << "    ◆ " << text << RST << "\n";
            break;
        case 5:
        case 6:
            std::cout << BOLD << FG_BRIGHT_WHITE << "    · " << text << RST << "\n";
            break;
        default:
            std::cout << BOLD << text << RST << "\n";
    }
}

void TuiRenderer::renderParagraph(const std::shared_ptr<HtmlNode>& node) {
    std::string text = resolveInlineText(node);
    if (text.empty()) return;

    auto lines = wordWrap(text, termWidth - 4);
    std::cout << "\n";
    for (auto& ln : lines)
        std::cout << "  " << ln << "\n";
    std::cout << "\n";
}

void TuiRenderer::renderUnorderedList(const std::shared_ptr<HtmlNode>& node, int depth) {
    std::cout << "\n";
    for (auto& ch : node->children) {
        if (ch->tag == "li") renderListItem(ch, depth, "  • ");
    }
    std::cout << "\n";
}

void TuiRenderer::renderOrderedList(const std::shared_ptr<HtmlNode>& node, int depth) {
    std::cout << "\n";
    int n = 1;
    for (auto& ch : node->children) {
        if (ch->tag == "li") renderListItem(ch, depth, "  " + std::to_string(n++) + ". ");
    }
    std::cout << "\n";
}

void TuiRenderer::renderListItem(const std::shared_ptr<HtmlNode>& node, int depth, const std::string& bullet) {
    std::string text = resolveInlineText(node);
    std::string padding(depth * 2, ' ');

    auto lines = wordWrap(text, termWidth - (int)bullet.size() - (int)padding.size() - 2);
    bool first = true;
    for (auto& ln : lines) {
        if (first) {
            std::cout << padding << FG_BRIGHT_YELLOW << bullet << RST << ln << "\n";
            first = false;
        } else {
            std::cout << padding << std::string(bullet.size(), ' ') << ln << "\n";
        }
    }

    // Nested lists within the li
    for (auto& ch : node->children) {
        if (ch->tag == "ul") renderUnorderedList(ch, depth + 1);
        else if (ch->tag == "ol") renderOrderedList(ch, depth + 1);
    }
}

void TuiRenderer::renderTable(const std::shared_ptr<HtmlNode>& node) {
    // Collect all rows
    std::vector<std::shared_ptr<HtmlNode>> rows;
    bool hasHead = false;

    auto collectRows = [&](const std::shared_ptr<HtmlNode>& n) {
        for (auto& ch : n->children) {
            if (ch->tag == "tr") rows.push_back(ch);
            else if (ch->tag == "thead" || ch->tag == "tbody" || ch->tag == "tfoot") {
                if (ch->tag == "thead") hasHead = true;
                for (auto& r : ch->children)
                    if (r->tag == "tr") rows.push_back(r);
            }
        }
    };
    collectRows(node);
    if (rows.empty()) return;

    // Compute column count and widths
    size_t cols = 0;
    for (auto& row : rows) {
        size_t cnt = 0;
        for (auto& c : row->children)
            if (c->tag == "td" || c->tag == "th") cnt++;
        cols = std::max(cols, cnt);
    }
    if (cols == 0) return;

    int colW = (termWidth - (int)cols - 1) / (int)cols;
    if (colW < 5) colW = 5;

    std::cout << "\n";
    // Top border
    std::cout << FG_BRIGHT_CYAN << TL;
    for (size_t c = 0; c < cols; c++) {
        for (int i = 0; i < colW; i++) std::cout << HZ;
        std::cout << (c + 1 < cols ? TM : TR);
    }
    std::cout << RST << "\n";

    bool firstRow = true;
    for (auto& row : rows) {
        bool isHeader = hasHead && firstRow;

        // Collect cell texts
        std::vector<std::string> cells;
        for (auto& c : row->children) {
            if (c->tag == "td" || c->tag == "th") {
                std::string t = resolveInlineText(c);
                if ((int)t.size() > colW - 2) t = t.substr(0, colW - 3) + "…";
                cells.push_back(t);
            }
        }
        while (cells.size() < cols) cells.push_back("");

        // Row
        for (size_t c = 0; c < cols; c++) {
            std::cout << FG_BRIGHT_CYAN << VT << RST;
            if (isHeader) std::cout << BOLD << FG_BRIGHT_YELLOW;
            std::cout << " " << cells[c];
            int pad = colW - 1 - (int)cells[c].size();
            for (int i = 0; i < pad; i++) std::cout << ' ';
            if (isHeader) std::cout << RST;
        }
        std::cout << FG_BRIGHT_CYAN << VT << RST << "\n";

        // Separator after header
        if (firstRow && hasHead) {
            std::cout << FG_BRIGHT_CYAN << ML;
            for (size_t c = 0; c < cols; c++) {
                for (int i = 0; i < colW; i++) std::cout << HZ;
                std::cout << (c + 1 < cols ? XX : MR);
            }
            std::cout << RST << "\n";
        }
        firstRow = false;
    }

    // Bottom border
    std::cout << FG_BRIGHT_CYAN << BL;
    for (size_t c = 0; c < cols; c++) {
        for (int i = 0; i < colW; i++) std::cout << HZ;
        std::cout << (c + 1 < cols ? BM : BR);
    }
    std::cout << RST << "\n\n";
}

void TuiRenderer::renderInput(const std::shared_ptr<HtmlNode>& node) {
    std::string type  = node->attr("type", "text");
    std::string name  = node->attr("name", node->attr("id", "input"));
    std::string value = node->attr("value", "");
    std::string placeholder = node->attr("placeholder", "");

    if (type == "submit" || type == "button" || type == "reset") {
        std::string label = value.empty() ? name : value;
        std::cout << "  " << BOLD << BG_BLUE << FG_BRIGHT_WHITE
                  << " [ " << label << " ] " << RST << "\n";
    } else if (type == "checkbox") {
        std::string checked = node->attr("checked", "");
        std::cout << "  " << FG_BRIGHT_CYAN
                  << (checked == "true" ? "☑" : "☐") << RST
                  << " " << name << "\n";
    } else if (type == "radio") {
        std::cout << "  " << FG_BRIGHT_CYAN << "◉ " << RST << name << "\n";
    } else if (type == "color") {
        std::cout << "  " << FG_BRIGHT_MAGENTA << "🎨 " << RST << name
                  << " [" << (value.empty() ? "#000000" : value) << "]\n";
    } else {
        std::string display = value.empty() ? placeholder : value;
        // Text/password/email/etc.
        std::cout << "  " << DIM << name << RST << ": "
                  << FG_BRIGHT_WHITE << "[ " << display;
        int innerW = 30 - (int)display.size();
        for (int i = 0; i < innerW; i++) std::cout << '_';
        std::cout << " ]" << RST << "\n";
    }
}

void TuiRenderer::renderButton(const std::shared_ptr<HtmlNode>& node) {
    std::string label = getNodeText(node);
    if (label.empty()) label = node->attr("value", "Button");
    std::cout << "\n  " << BOLD << BG_BLUE << FG_BRIGHT_WHITE
              << "  [ " << label << " ]  " << RST << "\n\n";
}

void TuiRenderer::renderImage(const std::shared_ptr<HtmlNode>& node) {
    std::string src = node->attr("src", "");
    std::string alt = node->attr("alt", "[image]");
    std::string w   = node->attr("width", "");
    std::string h   = node->attr("height", "");

    std::cout << "  " << FG_BRIGHT_MAGENTA << "🖼  " << BOLD << alt << RST;
    if (!src.empty())
        std::cout << DIM << "  (" << src;
    if (!w.empty()) std::cout << " " << w << "px";
    if (!h.empty()) std::cout << "×" << h << "px";
    if (!src.empty()) std::cout << ")";
    std::cout << RST << "\n";
}

void TuiRenderer::renderLink(const std::shared_ptr<HtmlNode>& node) {
    std::string href = node->attr("href", "#");
    std::string text = getNodeText(node);
    if (text.empty()) text = href;
    std::cout << UNDR << FG_BRIGHT_BLUE << text << RST
              << DIM << " [→ " << href << "]" << RST;
}

void TuiRenderer::renderCode(const std::shared_ptr<HtmlNode>& node) {
    std::string text = getNodeText(node);
    std::cout << BG_BLACK << FG_BRIGHT_GREEN << " " << text << " " << RST;
}

void TuiRenderer::renderPre(const std::shared_ptr<HtmlNode>& node) {
    std::string text = getNodeText(node);
    std::cout << "\n";
    std::cout << FG_BRIGHT_CYAN << " ┌─ code " << repeat(HZ, termWidth - 10) << "─┐\n" << RST;

    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        std::cout << FG_BRIGHT_CYAN << " │ " << RST
                  << BG_BLACK << FG_BRIGHT_GREEN << line;
        // Pad to right border
        int pad = termWidth - 4 - (int)line.size();
        for (int i = 0; i < pad; i++) std::cout << ' ';
        std::cout << RST << FG_BRIGHT_CYAN << " │" << RST << "\n";
    }
    std::cout << FG_BRIGHT_CYAN << " └" << repeat(HZ, termWidth - 3) << "┘\n" << RST;
    std::cout << "\n";
}

void TuiRenderer::renderBlockquote(const std::shared_ptr<HtmlNode>& node, int depth) {
    std::cout << "\n";
    auto lines = wordWrap(resolveInlineText(node), termWidth - 6);
    for (auto& ln : lines)
        std::cout << FG_BRIGHT_CYAN << "  ┃ " << RST << ITAL << ln << RST << "\n";
    std::cout << "\n";
}

void TuiRenderer::renderHr() {
    std::cout << "\n" << FG_BRIGHT_BLACK;
    for (int i = 0; i < termWidth; i++) std::cout << "─";
    std::cout << RST << "\n\n";
}

void TuiRenderer::renderForm(const std::shared_ptr<HtmlNode>& node, int depth) {
    std::string action = node->attr("action", "#");
    std::cout << "\n" << BOLD << FG_BRIGHT_YELLOW << "  ✦ Form";
    if (action != "#") std::cout << " → " << action;
    std::cout << RST << "\n";
    printHRule('-', termWidth - 4, FG_BRIGHT_BLACK);
    renderChildren(node, depth + 1);
    printHRule('-', termWidth - 4, FG_BRIGHT_BLACK);
}

void TuiRenderer::renderNav(const std::shared_ptr<HtmlNode>& node, int depth) {
    std::cout << "\n" << BOLD << BG_BLACK << FG_BRIGHT_CYAN << " Navigation " << RST << "\n";
    // Render nav links inline
    std::cout << "  ";
    bool first = true;
    std::function<void(const std::shared_ptr<HtmlNode>&)> extractLinks;
    extractLinks = [&](const std::shared_ptr<HtmlNode>& n) {
        if (!n) return;
        if (n->tag == "a") {
            if (!first) std::cout << FG_BRIGHT_BLACK << " │ " << RST;
            first = false;
            std::string text = getNodeText(n);
            if (text.empty()) text = n->attr("href", "link");
            std::cout << UNDR << FG_BRIGHT_BLUE << text << RST;
        }
        for (auto& ch : n->children) extractLinks(ch);
    };
    extractLinks(node);
    std::cout << "\n\n";
}

void TuiRenderer::renderHeader(const std::shared_ptr<HtmlNode>& node, int depth) {
    std::cout << BOLD << BG_BLUE << FG_BRIGHT_WHITE;
    std::cout << " ╔══ HEADER ";
    for (int i = 11; i < termWidth - 1; i++) std::cout << ' ';
    std::cout << RST << "\n";
    renderChildren(node, depth + 1);
    std::cout << FG_BRIGHT_BLUE;
    for (int i = 0; i < termWidth; i++) std::cout << "═";
    std::cout << RST << "\n\n";
}

void TuiRenderer::renderFooter(const std::shared_ptr<HtmlNode>& node, int depth) {
    std::cout << "\n" << FG_BRIGHT_BLUE;
    for (int i = 0; i < termWidth; i++) std::cout << "═";
    std::cout << RST << "\n";
    std::cout << DIM;
    renderChildren(node, depth + 1);
    std::cout << RST;
}

void TuiRenderer::renderSection(const std::shared_ptr<HtmlNode>& node, int depth) {
    std::cout << "\n";
    drawBox("section", resolveInlineText(node), FG_BRIGHT_BLACK, FG_WHITE, depth);
    renderChildren(node, depth + 1);
}

void TuiRenderer::renderDiv(const std::shared_ptr<HtmlNode>& node, int depth) {
    // If div has only text, render as paragraph-like
    if (node->children.empty() && !node->text.empty()) {
        auto lines = wordWrap(node->text, termWidth - 4);
        for (auto& ln : lines)
            std::cout << "  " << ln << "\n";
        return;
    }
    renderChildren(node, depth);
}

// ─── Main Node Router ───────────────────────────────────────────────────────
void TuiRenderer::renderNode(const std::shared_ptr<HtmlNode>& node, int depth, bool insideList) {
    if (!node) return;
    const std::string& t = node->tag;

    if (t == "#text")  { if (!node->text.empty()) std::cout << "  " << node->text << "\n"; }
    else if (t == "html" || t == "body") renderChildren(node, depth);
    else if (t == "head" || t == "script" || t == "style") { /* skip rendering */ }
    else if (t == "title") {
        std::string title = getNodeText(node);
        std::cout << BOLD << BG_MAGENTA << FG_BRIGHT_WHITE
                  << "  📄 " << title << "  " << RST << "\n\n";
    }
    else if (t == "h1") renderHeading(node, 1);
    else if (t == "h2") renderHeading(node, 2);
    else if (t == "h3") renderHeading(node, 3);
    else if (t == "h4") renderHeading(node, 4);
    else if (t == "h5") renderHeading(node, 5);
    else if (t == "h6") renderHeading(node, 6);
    else if (t == "p")  renderParagraph(node);
    else if (t == "ul") renderUnorderedList(node, depth);
    else if (t == "ol") renderOrderedList(node, depth);
    else if (t == "li") renderListItem(node, depth, "  • ");
    else if (t == "table") renderTable(node);
    else if (t == "pre")   renderPre(node);
    else if (t == "blockquote") renderBlockquote(node, depth);
    else if (t == "hr")  renderHr();
    else if (t == "br")  std::cout << "\n";
    else if (t == "form") renderForm(node, depth);
    else if (t == "input") renderInput(node);
    else if (t == "textarea") {
        std::string name = node->attr("name", node->attr("id", "textarea"));
        std::string rows = node->attr("rows", "3");
        std::cout << "  " << DIM << name << RST << ":\n";
        int r = std::atoi(rows.c_str());
        for (int i = 0; i < r; i++) {
            std::cout << "  │" << std::string(60, ' ') << "│\n";
        }
    }
    else if (t == "select") {
        std::string name = node->attr("name", node->attr("id", "select"));
        std::cout << "  " << DIM << name << RST << ": [ ▼ ";
        if (!node->children.empty() && node->children[0]->tag == "option")
            std::cout << getNodeText(node->children[0]);
        std::cout << " ]\n";
    }
    else if (t == "button") renderButton(node);
    else if (t == "img")    renderImage(node);
    else if (t == "a")      { renderLink(node); std::cout << "\n"; }
    else if (t == "code")   { renderCode(node); std::cout << "\n"; }
    else if (t == "nav")    renderNav(node, depth);
    else if (t == "header") renderHeader(node, depth);
    else if (t == "footer") renderFooter(node, depth);
    else if (t == "section" || t == "article") renderChildren(node, depth);
    else if (t == "main" || t == "aside") renderChildren(node, depth);
    else if (t == "div" || t == "span" || t == "label") renderDiv(node, depth);
    else if (t == "strong" || t == "b") {
        std::cout << BOLD << getNodeText(node) << RST << "\n";
    }
    else if (t == "em" || t == "i") {
        std::cout << ITAL << getNodeText(node) << RST << "\n";
    }
    else if (t == "u") {
        std::cout << UNDR << getNodeText(node) << RST << "\n";
    }
    else if (t == "abbr" || t == "time" || t == "small") {
        std::cout << DIM << getNodeText(node) << RST << "\n";
    }
    else if (t == "mark") {
        std::cout << BG_YELLOW << FG_BLACK << getNodeText(node) << RST << "\n";
    }
    else {
        // Unknown tag - just render children / text
        if (!node->text.empty()) std::cout << "  " << node->text << "\n";
        renderChildren(node, depth);
    }
}

void TuiRenderer::renderChildren(const std::shared_ptr<HtmlNode>& node, int depth, bool insideList) {
    for (auto& ch : node->children) {
        renderNode(ch, depth, insideList);
    }
}

// ─── Main Entry Point ──────────────────────────────────────────────────────
void TuiRenderer::render(const std::vector<std::shared_ptr<HtmlNode>>& roots) {
    clearScreen();

    // Status bar at top
    std::cout << BOLD << BG_BRIGHT_BLUE << FG_BRIGHT_WHITE;
    std::cout << " WebC TUI Browser ";
    for (int i = 18; i < termWidth - 1; i++) std::cout << ' ';
    std::cout << RST << "\n";

    for (auto& root : roots) {
        renderNode(root, 0);
    }

    // Status bar at bottom
    std::cout << "\n" << BOLD << BG_BRIGHT_BLUE << FG_BRIGHT_WHITE;
    std::cout << " Press Ctrl+C to exit ";
    for (int i = 22; i < termWidth - 1; i++) std::cout << ' ';
    std::cout << RST << "\n";

    std::cout.flush();
}
