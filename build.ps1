# WebC — Build Script (Windows / MinGW g++)
param()

chcp 65001 | Out-Null

$Root     = Split-Path -Parent $MyInvocation.MyCommand.Path
$SrcDir   = "$Root\src"
$OutExe   = "$SrcDir\webc.exe"
$InputDir = "$Root\input"

Write-Host ""
Write-Host "  WebC -- Compiler Design Project" -ForegroundColor Cyan
Write-Host "  ================================" -ForegroundColor Cyan
Write-Host ""

# Validate all source files exist
$sources = @(
    "src/html/HtmlParser.cpp",
    "src/renderer/TuiRenderer.cpp",
    "src/lexer/Lexer.cpp",
    "src/parser/Parser.cpp",
    "src/semantics/SemanticAnalyzer.cpp",
    "src/codegen/HtmlNodeGen.cpp",
    "src/main.cpp"
)

$missing = $false
foreach ($s in $sources) {
    if (-not (Test-Path (Join-Path $Root $s))) {
        Write-Host "  [ERROR] Missing: $s" -ForegroundColor Red
        $missing = $true
    }
}
if ($missing) { exit 1 }

Write-Host "  [1/2] Compiling $($sources.Count) files..." -ForegroundColor Yellow

# Build the full command as a single string and invoke it
$srcList = ($sources -join " ")
$cmd = "g++ -std=c++17 -O2 -I`"src`" $srcList -o `"$OutExe`""

Push-Location $Root
$result = Invoke-Expression "$cmd 2>&1"
$code = $LASTEXITCODE
Pop-Location

if ($result) { Write-Host $result }

if ($code -ne 0) {
    Write-Host ""
    Write-Host "  [FAIL] Build failed." -ForegroundColor Red
    exit 1
}

Write-Host "  [2/2] Build successful!  ->  src\webc.exe" -ForegroundColor Green
Write-Host ""

# Create input/ if missing
if (-not (Test-Path $InputDir)) {
    New-Item -ItemType Directory -Path $InputDir | Out-Null
    Write-Host "  Created: input/" -ForegroundColor Green
}

# Auto-run: prefer .webc to demo the compiler pipeline
$webcFiles = Get-ChildItem -Path $InputDir -Filter "*.webc" -ErrorAction SilentlyContinue
$htmlFiles = Get-ChildItem -Path $InputDir -Filter "*.html" -ErrorAction SilentlyContinue

$target = if ($webcFiles) { $webcFiles[0] } elseif ($htmlFiles) { $htmlFiles[0] } else { $null }

if ($target) {
    Write-Host "  [RUN] $($target.Name)" -ForegroundColor Cyan
    Write-Host ""
    & $OutExe $target.FullName
} else {
    Write-Host "  TIP: Add .webc or .html files to input/ then run:" -ForegroundColor Yellow
    Write-Host '        .\src\webc.exe input\hello.webc' -ForegroundColor White
    Write-Host '        .\src\webc.exe input\demo.html'  -ForegroundColor White
}
