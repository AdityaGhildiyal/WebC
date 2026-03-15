# WebC — A Compiler Design Project

WebC is a compiler that takes `.webc` source files (an HTML/JS hybrid language) and renders the evaluated output as a rich **Terminal UI (TUI)** — right in your terminal window, no browser required.

It also doubles as a TUI HTML browser, accepting plain `.html` files directly.

---

## What Makes This a Compiler?

WebC implements the complete classical compiler pipeline:

```
Source (.webc)
      │
      ▼
┌─────────────┐
│    Lexer     │  Tokenizes source into keywords, identifiers, literals, operators
└──────┬──────┘  src/lexer/
       │
       ▼
┌─────────────┐
│   Parser    │  Recursive-descent parser → builds an Abstract Syntax Tree (AST)
└──────┬──────┘  src/parser/
       │
       ▼
┌──────────────────┐
│ Semantic Analyzer│  Checks scoping, variable declarations, type consistency
└──────┬───────────┘  src/semantics/
       │
       ▼
┌──────────────┐
│  HtmlNodeGen │  Compiler backend — evaluates the AST, resolves all expressions,
└──────┬───────┘  produces an HtmlNode DOM tree        src/codegen/
       │
       ▼
┌──────────────┐
│ TUI Renderer │  Runtime — renders the DOM tree to the terminal via ANSI codes
└──────────────┘  src/renderer/
```

For plain `.html` files, the pipeline short-circuits directly to:
```
HTML file  →  HtmlParser  →  HtmlNode tree  →  TUI Renderer
```

---

## The WebC Language

WebC lets you embed JavaScript-style logic directly inside HTML structure:

```webc
<html>
<body>
  <h1 id="title">"My App"</h1>

  <section id="calc">
    <h2>"Pricing"</h2>

    let price    = 299;
    let qty      = 3;
    let subtotal = price * qty;
    let tax      = subtotal * 18 / 100;
    let total    = subtotal + tax;

    <ul>
      <li>"Subtotal : $" subtotal</li>
      <li>"Tax (18%): $" tax</li>
      <li>"Total    : $" total</li>
    </ul>
  </section>
</body>
</html>
```

The compiler evaluates `subtotal`, `tax`, and `total` at **compile time** and injects the results into the TUI output.

### Supported WebC Syntax

| Feature | Example |
|---|---|
| HTML tags | `<div id="app">` … `</div>` |
| String content | `"Hello, World!"` |
| Variable declaration | `let x = 10;` / `const y = 3.14;` |
| Arithmetic | `let area = pi * r * r;` |
| Variable reference | `"Total: " total` |
| HTML nesting | `<ul><li>"item"</li></ul>` |

### Supported HTML Elements (TUI Renderer)

`h1`–`h6` · `p` · `div` · `span` · `ul` / `ol` / `li` · `table` / `thead` / `tbody` / `tr` / `th` / `td` · `pre` · `code` · `blockquote` · `hr` · `a` · `img` · `form` · `input` · `textarea` · `select` · `button` · `nav` · `header` · `footer` · `section` · `article` · `main` · `strong` · `em` · `u` · `mark`

---

## Project Structure

```
WebC/
├── build.ps1                  ← One-command build script (Windows/MinGW)
├── README.md
├── input/                     ← Drop your .webc and .html files here
│   ├── hello.webc             ← Sample WebC program (compiler pipeline)
│   ├── demo.html              ← Sample HTML file (HTML pipeline)
│   └── portfolio.html         ← Rich HTML example
└── src/
    ├── main.cpp               ← Entry point — routes .webc vs .html
    ├── lexer/
    │   ├── Lexer.hpp
    │   └── Lexer.cpp          ← Tokenizer
    ├── parser/
    │   ├── Parser.hpp
    │   └── Parser.cpp         ← Recursive-descent parser
    ├── ast/
    │   └── ASTNodes.hpp       ← AST node types
    ├── semantics/
    │   ├── SemanticAnalyzer.hpp
    │   ├── SemanticAnalyzer.cpp
    │   └── SymbolTable.hpp    ← Scope-aware symbol table
    ├── codegen/
    │   ├── HtmlNodeGen.hpp
    │   └── HtmlNodeGen.cpp    ← Compiler backend (AST → HtmlNode tree)
    ├── html/
    │   ├── HtmlNode.hpp       ← DOM node type
    │   ├── HtmlParser.hpp
    │   └── HtmlParser.cpp     ← Real HTML parser
    └── renderer/
        ├── TuiRenderer.hpp
        └── TuiRenderer.cpp    ← Terminal UI renderer (the runtime)
```

---

## Requirements

- **g++** with C++17 support (MinGW on Windows, GCC on Linux/Mac)
- Windows: PowerShell 5+ for `build.ps1`

No LLVM, no CMake, no third-party libraries required.

---

## How to Build

```powershell
.\build.ps1
```

This compiles all source files and produces `src/webc.exe`. It will also auto-run the first `.webc` or `.html` file found in `input/`.

To build manually:

```bash
g++ -std=c++17 -O2 -I"src" \
    src/html/HtmlParser.cpp \
    src/renderer/TuiRenderer.cpp \
    src/lexer/Lexer.cpp \
    src/parser/Parser.cpp \
    src/semantics/SemanticAnalyzer.cpp \
    src/codegen/HtmlNodeGen.cpp \
    src/main.cpp \
    -o src/webc.exe
```

---

## How to Run

### Compile and render a `.webc` file (full compiler pipeline)

```powershell
.\src\webc.exe input\hello.webc
```

### Render a plain `.html` file (TUI browser mode)

```powershell
.\src\webc.exe input\demo.html
.\src\webc.exe input\portfolio.html
```

### Render all files in the `input/` folder

```powershell
.\src\webc.exe
```

### Render files from a custom folder

```powershell
.\src\webc.exe --dir C:\path\to\myfolder
```

### Pass any `.html` file directly

```powershell
.\src\webc.exe C:\Users\you\Downloads\page.html
```

---

## Examples

### Running the sample WebC program

```
.\src\webc.exe input\hello.webc
```

Expected output (TUI):

```
 WebC TUI Browser ──────────────────────────────────────────────────────
 ╔══ HEADER ──────────────────────────────────────────────────────────
  WebC Live App

  Navigation    Dashboard │ Analytics │ Settings

  Dynamic Calculations
  ────────────────────

  ▶ Screen Stats

  • Resolution : 1920 x 1080
  • Total pixels: 2073600
  • Bandwidth   : 497664000 bytes/sec

  Shopping Cart
  ─────────────
  ┌──────────────────┬──────────────────┬──────────┬───────────┐
  │ Item             │ Unit Price       │ Qty      │ Amount    │
  ├──────────────────┼──────────────────┼──────────┼───────────┤
  │ WebC Pro License │ $299             │ 3        │ $897      │
  │ Tax (18%)        │                  │          │ $152.46   │
  │ TOTAL            │                  │          │ $999.46   │
  └──────────────────┴──────────────────┴──────────┴───────────┘
```

---

## Language Grammar (simplified)

```
program     := node*
node        := tag | statement | expression
tag         := '<' IDENT attrs '>' node* '</' IDENT '>'
attrs       := (IDENT ('=' STRING)?)*
statement   := ('let' | 'const') IDENT '=' expression ';'
            |  IDENT '=' expression ';'
expression  := comparison (('&&' | '||') comparison)*
comparison  := term ((== | != | < | > | <= | >=) term)*
term        := factor (('+' | '-') factor)*
factor      := primary (('*' | '/') primary)*
primary     := NUMBER | STRING | IDENT | '(' expression ')'
```

---

## Acknowledgements

Built from scratch in **C++17** — no parser generators, no external libraries, no browser engines.

- Lexer: hand-written character-by-character scanner
- Parser: hand-written recursive descent (LL(1))
- Semantic Analyzer: single-pass with a scoped symbol table
- Backend: tree-walking interpreter producing a DOM
- Runtime: ANSI escape code TUI renderer with box-drawing characters
