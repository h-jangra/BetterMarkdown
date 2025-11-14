#include <windows.h>
#include <string>
#include <vector>
#include <map>

#include "../plugin/PluginInterface.h"
#include "../plugin/Scintilla.h"
#include "../plugin/Notepad_plus_msgs.h"

HINSTANCE g_hInstance = nullptr;
NppData nppData;

const TCHAR PLUGIN_NAME[] = TEXT("MarkdownStyler");
const int nbFunc = 3;
FuncItem funcItem[nbFunc];

// Markdown style configuration
struct MarkdownStyle {
    int size;
    COLORREF color;
    bool bold;
    bool italic;
};

std::map<int, MarkdownStyle> g_markdownStyles;

// Function declarations
void applyMarkdownStyles();
void showStyleConfig();
void about();
bool isMarkdownFile();
void setupDefaultStyles();

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID /*lpReserved*/) {
    if (reasonForCall == DLL_PROCESS_ATTACH) {
        g_hInstance = (HINSTANCE)hModule;
        setupDefaultStyles();
    }
    return TRUE;
}

void setupDefaultStyles() {
    // Headers
    g_markdownStyles[1] = { 18, RGB(0, 0, 139), true, false };  // H1 - Dark Blue
    g_markdownStyles[2] = { 16, RGB(0, 100, 0), true, false };  // H2 - Dark Green
    g_markdownStyles[3] = { 14, RGB(139, 0, 0), true, false };  // H3 - Dark Red
    g_markdownStyles[4] = { 12, RGB(128, 0, 128), true, false };// H4 - Purple
    
    // Code
    g_markdownStyles[5] = { 10, RGB(165, 42, 42), false, false }; // Inline code
    g_markdownStyles[6] = { 10, RGB(0, 0, 0), false, true };     // Block quotes
    g_markdownStyles[7] = { 10, RGB(255, 0, 0), false, false };  // Links
    g_markdownStyles[8] = { 10, RGB(0, 100, 0), true, false };   // Bold
    g_markdownStyles[9] = { 10, RGB(0, 100, 0), false, true };   // Italic
}

bool isMarkdownFile() {
    TCHAR filePath[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM)filePath);
    
    std::wstring path(filePath);
    if (path.length() < 3) return false;
    
    std::wstring ext = path.substr(path.length() - 3);
    return (ext == L".md" || ext == L".MD");
}

void applyMarkdownStyles() {
    if (!isMarkdownFile()) {
        ::MessageBox(nppData._nppHandle, 
            TEXT("This is not a Markdown file (.md)"), 
            TEXT("Markdown Styler"), MB_OK | MB_ICONINFORMATION);
        return;
    }

    HWND hScintilla = (HWND)::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, 0);
    if (!hScintilla) return;

    // Clear existing styles
    ::SendMessage(hScintilla, SCI_STYLECLEARALL, 0, 0);

    // Set default style
    ::SendMessage(hScintilla, SCI_STYLESETFORE, STYLE_DEFAULT, RGB(0, 0, 0));
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, STYLE_DEFAULT, 11);
    ::SendMessage(hScintilla, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");
    
    // Apply lexer for basic syntax highlighting
    ::SendMessage(hScintilla, SCI_SETLEXER, SCLEX_MARKDOWN, 0);
    
    // Configure styles for different markdown elements
    for (const auto& style : g_markdownStyles) {
        ::SendMessage(hScintilla, SCI_STYLESETSIZE, style.first, style.second.size);
        ::SendMessage(hScintilla, SCI_STYLESETFORE, style.first, style.second.color);
        ::SendMessage(hScintilla, SCI_STYLESETBOLD, style.first, style.second.bold);
        ::SendMessage(hScintilla, SCI_STYLESETITALIC, style.first, style.second.italic);
    }
    
    // Refresh the view
    ::SendMessage(hScintilla, SCI_COLOURISE, 0, -1);
    
    ::MessageBox(nppData._nppHandle, 
        TEXT("Markdown styles applied successfully!"), 
        TEXT("Markdown Styler"), MB_OK | MB_ICONINFORMATION);
}

// Simple configuration dialog
INT_PTR CALLBACK StyleConfigDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        return TRUE;
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            // Save configuration here
            EndDialog(hwnd, IDOK);
            return TRUE;
            
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

void showStyleConfig() {
    DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_CONFIG), nppData._nppHandle, StyleConfigDialog);
}

void about() {
    ::MessageBox(nppData._nppHandle,
        TEXT("Markdown Styler Plugin\n\n")
        TEXT("A simple plugin to enhance Markdown file styling in Notepad++\n")
        TEXT("Features:\n")
        TEXT("• Bigger, colored headings\n")
        TEXT("• Syntax highlighting for markdown elements\n")
        TEXT("• Customizable colors and sizes"),
        TEXT("About Markdown Styler"), MB_OK | MB_ICONINFORMATION);
}

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData) {
    nppData = notpadPlusData;
}

extern "C" __declspec(dllexport) const TCHAR* getName() {
    return PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* nbF) {
    *nbF = nbFunc;

    lstrcpy(funcItem[0]._itemName, TEXT("Apply Markdown Styles"));
    funcItem[0]._pFunc = applyMarkdownStyles;
    funcItem[0]._pShKey = NULL;
    funcItem[0]._init2Check = false;

    lstrcpy(funcItem[1]._itemName, TEXT("Style Configuration"));
    funcItem[1]._pFunc = showStyleConfig;
    funcItem[1]._pShKey = NULL;
    funcItem[1]._init2Check = false;

    lstrcpy(funcItem[2]._itemName, TEXT("About"));
    funcItem[2]._pFunc = about;
    funcItem[2]._pShKey = NULL;
    funcItem[2]._init2Check = false;

    return funcItem;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification* notifyCode) {
    if (!notifyCode) return;

    switch (notifyCode->nmhdr.code) {
    case NPPN_BUFFERACTIVATED:
        // Auto-apply styles when switching to markdown files
        if (isMarkdownFile()) {
            applyMarkdownStyles();
        }
        break;
        
    case NPPN_FILEOPENED:
        if (isMarkdownFile()) {
            applyMarkdownStyles();
        }
        break;
    }
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam) {
    return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode() {
    return TRUE;
}
#endif
