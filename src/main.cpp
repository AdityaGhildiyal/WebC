// ============================================================
//  WebC — Unified Entry Point
//  Supports two modes:
//    .html  →  HtmlParser → HtmlNode tree → TuiRenderer
//    .webc  →  Lexer → Parser → SemanticAnalyzer
//                → HtmlNodeGen → HtmlNode tree → TuiRenderer
// ============================================================
#include "html/HtmlParser.hpp"
#include "renderer/TuiRenderer.hpp"
#include "lexer/Lexer.hpp"
#include "parser/Parser.hpp"
#include "semantics/SemanticAnalyzer.hpp"
#include "codegen/HtmlNodeGen.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>
#ifdef _WIN32
#  include <windows.h>
#endif

// ─── ANSI helpers ─────────────────────────────────────────
#define RST    "\033[0m"
#define BOLD   "\033[1m"
#define DIM    "\033[2m"
#define RED    "\033[31m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define CYAN   "\033[36m"
#define MAGENTA "\033[35m"

// ─── Utilities ────────────────────────────────────────────
static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("Cannot open: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static void listHtmlFiles(const std::string& dir, std::vector<std::string>& out) {
#ifdef _WIN32
    WIN32_FIND_DATAA fd;
    // .html files
    HANDLE h1 = FindFirstFileA((dir + "\\*.html").c_str(), &fd);
    if (h1 != INVALID_HANDLE_VALUE) {
        do { out.push_back(dir + "\\" + fd.cFileName); }
        while (FindNextFileA(h1, &fd));
        FindClose(h1);
    }
    // .webc files
    HANDLE h2 = FindFirstFileA((dir + "\\*.webc").c_str(), &fd);
    if (h2 != INVALID_HANDLE_VALUE) {
        do { out.push_back(dir + "\\" + fd.cFileName); }
        while (FindNextFileA(h2, &fd));
        FindClose(h2);
    }
#else
    FILE* p = popen(("ls \""+dir+"\"/*.html \""+dir+"\"/*.webc 2>/dev/null").c_str(), "r");
    if (!p) return;
    char buf[512];
    while (fgets(buf, sizeof(buf), p)) {
        std::string n(buf);
        while (!n.empty() && (n.back()=='\n'||n.back()=='\r')) n.pop_back();
        if (!n.empty()) out.push_back(n);
    }
    pclose(p);
#endif
}

// ─── Compile & Render one .webc file ─────────────────────
static void renderWebc(const std::string& path, TuiRenderer& renderer) {
    std::cout << CYAN << "  [WebC] " << RST << path << "\n";

    std::string src = readFile(path);

    // 1. Lex
    Lexer lexer(src);
    auto tokens = lexer.tokenize();

    // 2. Parse
    Parser parser(tokens);
    auto ast = parser.parseProgram();

    // 3. Semantic analysis (warn on errors but keep going)
    try {
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast);
    } catch (const std::exception& e) {
        std::cerr << YELLOW << "  [Semantic Warning] " << e.what() << RST << "\n";
    }

    // 4. Generate HtmlNode tree (our new backend)
    HtmlNodeGen gen;
    auto nodes = gen.generate(ast);

    // 5. Render
    renderer.render(nodes);
}

// ─── Parse & Render one .html file ───────────────────────
static void renderHtml(const std::string& path, TuiRenderer& renderer) {
    std::cout << CYAN << "  [HTML] " << RST << path << "\n";
    std::string src = readFile(path);
    HtmlParser parser(src);
    auto nodes = parser.parse();
    renderer.render(nodes);
}

// ─── Main ────────────────────────────────────────────────
int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Set console output to UTF-8 so box-drawing characters display correctly
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::string inputDir   = "input";
    std::string singleFile = "";

    if (argc == 2) {
        std::string arg = argv[1];
        if (endsWith(arg, ".html") || endsWith(arg, ".webc"))
            singleFile = arg;
        else
            inputDir = arg;
    } else if (argc == 3 && std::string(argv[1]) == "--dir") {
        inputDir = argv[2];
    }

    std::vector<std::string> files;
    if (!singleFile.empty()) {
        files.push_back(singleFile);
    } else {
        listHtmlFiles(inputDir, files);
    }

    if (files.empty()) {
        std::cerr << RED << "Error: No .html or .webc files found in '"
                  << inputDir << "/'\n" << RST;
        std::cerr << YELLOW
            << "Usage:\n"
            << "  " << argv[0] << " path/to/file.html      (render HTML)\n"
            << "  " << argv[0] << " path/to/file.webc      (compile & render WebC)\n"
            << "  " << argv[0] << "                         (all files in input/)\n"
            << RST;
        return 1;
    }

    TuiRenderer renderer;

    for (size_t i = 0; i < files.size(); i++) {
        const auto& path = files[i];
        try {
            if (endsWith(path, ".webc"))
                renderWebc(path, renderer);
            else
                renderHtml(path, renderer);

            // Pause between multiple files
            if (files.size() > 1 && i + 1 < files.size()) {
                std::cout << YELLOW
                          << "\n[" << (i+1) << "/" << files.size()
                          << "] Press ENTER for next file..."
                          << RST;
                std::cin.ignore();
            }
        } catch (const std::exception& e) {
            std::cerr << RED << "Error in '" << path << "': "
                      << e.what() << RST << "\n";
        }
    }

    return 0;
}
