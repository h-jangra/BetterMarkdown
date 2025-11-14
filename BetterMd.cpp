#include <windows.h>
#include <string>
#include <algorithm>

#include "plugin/PluginInterface.h"
#include "plugin/Scintilla.h"
#include "plugin/Notepad_plus_msgs.h"

// Scintilla constants
#ifndef SCI_SETLEXER
#define SCI_SETLEXER 4001
#endif
#ifndef SCLEX_MARKDOWN
#define SCLEX_MARKDOWN 55
#endif

// Markdown lexer styles
#define SCE_MARKDOWN_DEFAULT 0
#define SCE_MARKDOWN_LINE_BEGIN 1
#define SCE_MARKDOWN_STRONG1 2
#define SCE_MARKDOWN_STRONG2 3
#define SCE_MARKDOWN_EM1 4
#define SCE_MARKDOWN_EM2 5
#define SCE_MARKDOWN_HEADER1 6
#define SCE_MARKDOWN_HEADER2 7
#define SCE_MARKDOWN_HEADER3 8
#define SCE_MARKDOWN_HEADER4 9
#define SCE_MARKDOWN_HEADER5 10
#define SCE_MARKDOWN_HEADER6 11
#define SCE_MARKDOWN_PRECHAR 12
#define SCE_MARKDOWN_ULIST_ITEM 13
#define SCE_MARKDOWN_OLIST_ITEM 14
#define SCE_MARKDOWN_BLOCKQUOTE 15
#define SCE_MARKDOWN_STRIKEOUT 16
#define SCE_MARKDOWN_HRULE 17
#define SCE_MARKDOWN_LINK 18
#define SCE_MARKDOWN_CODE 19
#define SCE_MARKDOWN_CODE2 20
#define SCE_MARKDOWN_CODEBK 21

const TCHAR NPP_PLUGIN_NAME[] = TEXT("Better Markdown");
const int nbFunc = 3;

FuncItem funcItem[nbFunc];
NppData nppData;
HINSTANCE _gModule;

bool g_stylesEnabled = true;

// Function declarations
void pluginInit(HANDLE hModule);
void pluginCleanUp();
void commandMenuInit();
void commandMenuCleanUp();
void toggleStyles();
void resetStyles();
void about();
bool isMarkdownFile();
HWND getCurrentScintilla();
void applyMarkdownStyles();

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPARAM /*lpReserved*/)
{
    switch (reasonForCall)
    {
    case DLL_PROCESS_ATTACH:
        pluginInit(hModule);
        break;
    case DLL_PROCESS_DETACH:
        pluginCleanUp();
        break;
    }
    return TRUE;
}

void pluginInit(HANDLE hModule)
{
    _gModule = (HINSTANCE)hModule;
}

void pluginCleanUp()
{
}

void commandMenuInit()
{
}

void commandMenuCleanUp()
{
}

HWND getCurrentScintilla()
{
    int which = 0;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    return (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
}

bool isMarkdownFile()
{
    TCHAR filePath[MAX_PATH] = {0};
    ::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM)filePath);

    if (filePath[0] == 0) return false;

#ifdef UNICODE
    std::wstring path(filePath);
    std::transform(path.begin(), path.end(), path.begin(), ::towlower);
    size_t len = path.length();

    if (len >= 3 && path.compare(len - 3, 3, L".md") == 0) return true;
    if (len >= 4 && path.compare(len - 4, 4, L".mkd") == 0) return true;
    if (len >= 9 && path.compare(len - 9, 9, L".markdown") == 0) return true;
#else
    std::string path(filePath);
    std::transform(path.begin(), path.end(), path.begin(), ::tolower);
    size_t len = path.length();

    if (len >= 3 && path.compare(len - 3, 3, ".md") == 0) return true;
    if (len >= 4 && path.compare(len - 4, 4, ".mkd") == 0) return true;
    if (len >= 9 && path.compare(len - 9, 9, ".markdown") == 0) return true;
#endif

    return false;
}

void applyMarkdownStyles()
{
    if (!isMarkdownFile()) return;

    HWND hScintilla = getCurrentScintilla();
    if (!hScintilla) return;

    // Enable Markdown lexer
    ::SendMessage(hScintilla, SCI_SETLEXER, SCLEX_MARKDOWN, 0);

    // Detect dark mode
    COLORREF defaultBg = (COLORREF)::SendMessage(hScintilla, SCI_STYLEGETBACK, STYLE_DEFAULT, 0);
    int r = GetRValue(defaultBg);
    int g = GetGValue(defaultBg);
    int b = GetBValue(defaultBg);
    bool isDark = ((r + g + b) / 3) < 128;

    // Theme-aware colors
    COLORREF h1Color = isDark ? RGB(255, 105, 120) : RGB(220, 20, 60);      // Crimson Red
    COLORREF h2Color = isDark ? RGB(100, 180, 255) : RGB(30, 144, 255);     // Dodger Blue
    COLORREF h3Color = isDark ? RGB(100, 220, 130) : RGB(34, 139, 34);      // Forest Green
    COLORREF h4Color = isDark ? RGB(255, 180, 80) : RGB(255, 140, 0);       // Dark Orange
    COLORREF h5Color = isDark ? RGB(200, 130, 255) : RGB(138, 43, 226);     // Blue Violet
    COLORREF h6Color = isDark ? RGB(255, 130, 90) : RGB(178, 34, 34);       // Firebrick

    COLORREF codeColor = isDark ? RGB(255, 150, 200) : RGB(199, 37, 78);
    COLORREF codeBg = isDark ? RGB(50, 50, 55) : RGB(245, 245, 248);

    COLORREF quoteColor = isDark ? RGB(180, 180, 190) : RGB(100, 100, 110);
    COLORREF quoteBg = isDark ? RGB(45, 45, 50) : RGB(250, 250, 245);

    COLORREF linkColor = isDark ? RGB(100, 180, 255) : RGB(0, 102, 204);
    COLORREF listColor = isDark ? RGB(120, 200, 255) : RGB(0, 120, 215);
    COLORREF ruleColor = isDark ? RGB(120, 120, 130) : RGB(180, 180, 190);

    COLORREF boldColor = isDark ? RGB(255, 130, 140) : RGB(200, 0, 20);
    COLORREF italicColor = isDark ? RGB(130, 220, 150) : RGB(20, 140, 40);
    COLORREF strikeColor = isDark ? RGB(140, 140, 150) : RGB(130, 130, 140);

    // H1 - 24pt, Extra Bold
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_HEADER1, 24);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_HEADER1, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_HEADER1, h1Color);

    // H2 - 22pt, Bold
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_HEADER2, 22);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_HEADER2, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_HEADER2, h2Color);

    // H3 - 20pt, Bold
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_HEADER3, 20);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_HEADER3, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_HEADER3, h3Color);

    // H4 - 18pt, Bold
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_HEADER4, 18);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_HEADER4, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_HEADER4, h4Color);

    // H5 - 16pt, Bold
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_HEADER5, 16);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_HEADER5, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_HEADER5, h5Color);

    // H6 - 14pt, Bold
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_HEADER6, 14);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_HEADER6, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_HEADER6, h6Color);

    // Bold text (**text** or __text__)
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_STRONG1, boldColor);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_STRONG1, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_STRONG2, boldColor);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_STRONG2, TRUE);

    // Italic text (*text* or _text_)
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_EM1, italicColor);
    ::SendMessage(hScintilla, SCI_STYLESETITALIC, SCE_MARKDOWN_EM1, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_EM2, italicColor);
    ::SendMessage(hScintilla, SCI_STYLESETITALIC, SCE_MARKDOWN_EM2, TRUE);

    // Inline code (`code`)
    ::SendMessage(hScintilla, SCI_STYLESETBACK, SCE_MARKDOWN_CODE, codeBg);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_CODE, codeColor);
    ::SendMessage(hScintilla, SCI_STYLESETFONT, SCE_MARKDOWN_CODE, (LPARAM)"Consolas");
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_CODE, 10);

    ::SendMessage(hScintilla, SCI_STYLESETBACK, SCE_MARKDOWN_CODE2, codeBg);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_CODE2, codeColor);
    ::SendMessage(hScintilla, SCI_STYLESETFONT, SCE_MARKDOWN_CODE2, (LPARAM)"Consolas");
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_CODE2, 10);

    // Code blocks (```code```)
    ::SendMessage(hScintilla, SCI_STYLESETBACK, SCE_MARKDOWN_CODEBK, codeBg);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_CODEBK, isDark ? RGB(210, 210, 220) : RGB(70, 70, 80));
    ::SendMessage(hScintilla, SCI_STYLESETFONT, SCE_MARKDOWN_CODEBK, (LPARAM)"Consolas");
    ::SendMessage(hScintilla, SCI_STYLESETEOLFILLED, SCE_MARKDOWN_CODEBK, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_CODEBK, 10);

    // Prechar (indented code blocks)
    ::SendMessage(hScintilla, SCI_STYLESETBACK, SCE_MARKDOWN_PRECHAR, codeBg);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_PRECHAR, codeColor);
    ::SendMessage(hScintilla, SCI_STYLESETFONT, SCE_MARKDOWN_PRECHAR, (LPARAM)"Consolas");

    // Block quotes (> text)
    ::SendMessage(hScintilla, SCI_STYLESETBACK, SCE_MARKDOWN_BLOCKQUOTE, quoteBg);
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_BLOCKQUOTE, quoteColor);
    ::SendMessage(hScintilla, SCI_STYLESETITALIC, SCE_MARKDOWN_BLOCKQUOTE, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETEOLFILLED, SCE_MARKDOWN_BLOCKQUOTE, TRUE);

    // Links [text](url)
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_LINK, linkColor);
    ::SendMessage(hScintilla, SCI_STYLESETUNDERLINE, SCE_MARKDOWN_LINK, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_LINK, FALSE);

    // Horizontal rules (---, ***, ___)
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_HRULE, ruleColor);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_HRULE, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_HRULE, 12);

    // Unordered list items (-, *, +)
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_ULIST_ITEM, listColor);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_ULIST_ITEM, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_ULIST_ITEM, 12);

    // Ordered list items (1., 2., etc)
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_OLIST_ITEM, listColor);
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_OLIST_ITEM, TRUE);
    ::SendMessage(hScintilla, SCI_STYLESETSIZE, SCE_MARKDOWN_OLIST_ITEM, 12);

    // Strikethrough (~~text~~)
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_STRIKEOUT, strikeColor);
    ::SendMessage(hScintilla, SCI_STYLESETITALIC, SCE_MARKDOWN_STRIKEOUT, TRUE);

    // Line begin (# symbols themselves)
    ::SendMessage(hScintilla, SCI_STYLESETFORE, SCE_MARKDOWN_LINE_BEGIN, isDark ? RGB(100, 100, 110) : RGB(160, 160, 170));
    ::SendMessage(hScintilla, SCI_STYLESETBOLD, SCE_MARKDOWN_LINE_BEGIN, TRUE);

    // Apply the styling
    ::SendMessage(hScintilla, SCI_COLOURISE, 0, -1);
}

void toggleStyles()
{
    g_stylesEnabled = !g_stylesEnabled;

    // Update menu checkmark
    HMENU hMenu = ::GetMenu(nppData._nppHandle);
    if (hMenu) {
        int menuCount = ::GetMenuItemCount(hMenu);
        for (int i = 0; i < menuCount; i++) {
            TCHAR menuStr[256];
            ::GetMenuString(hMenu, i, menuStr, 256, MF_BYPOSITION);
            if (_tcsstr(menuStr, TEXT("Plugins")) != NULL) {
                HMENU pluginMenu = ::GetSubMenu(hMenu, i);
                if (pluginMenu) {
                    ::CheckMenuItem(pluginMenu, funcItem[0]._cmdID,
                                  MF_BYCOMMAND | (g_stylesEnabled ? MF_CHECKED : MF_UNCHECKED));
                }
                break;
            }
        }
    }

    if (g_stylesEnabled && isMarkdownFile()) {
        applyMarkdownStyles();
    } else {
        resetStyles();
    }
}

void resetStyles()
{
    HWND hScintilla = getCurrentScintilla();
    if (!hScintilla) return;

    // Reset to default Notepad++ markdown styling
    ::SendMessage(hScintilla, SCI_STYLECLEARALL, 0, 0);
    ::SendMessage(hScintilla, SCI_SETLEXER, SCLEX_MARKDOWN, 0);
    ::SendMessage(hScintilla, SCI_COLOURISE, 0, -1);
}

void about()
{
    ::MessageBox(nppData._nppHandle,
        TEXT("Better Markdown Plugin v0.1\n\n")
        TEXT("Enhanced Markdown styling for Notepad++\n\n")
        TEXT("✨ Features:\n")
        TEXT("• Extra large headings (H1: 24pt → H6: 14pt)\n")
        TEXT("• Vibrant, colorful heading text\n")
        TEXT("• Background highlights for code blocks\n")
        TEXT("• Background highlights for quotes\n")
        TEXT("• Styled bold, italic, strikethrough\n")
        TEXT("• Underlined links with color\n")
        TEXT("• Larger, bold list markers\n")
        TEXT("• Enhanced horizontal rules\n")
        TEXT("• Automatic dark mode detection\n\n")
        TEXT("📝 Supported: .md, .mkd, .markdown\n\n")
        TEXT("Toggle styles from Plugins menu!"),
        TEXT("About Better Markdown"), MB_OK | MB_ICONINFORMATION);
}

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
    nppData = notpadPlusData;
    commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR* getName()
{
    return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* nbF)
{
    *nbF = nbFunc;

    lstrcpy(funcItem[0]._itemName, TEXT("Enable Better Styles"));
    funcItem[0]._pFunc = toggleStyles;
    funcItem[0]._init2Check = true;  // Show checkmark initially
    funcItem[0]._pShKey = NULL;

    lstrcpy(funcItem[1]._itemName, TEXT("Reset to Default"));
    funcItem[1]._pFunc = resetStyles;
    funcItem[1]._init2Check = false;
    funcItem[1]._pShKey = NULL;

    lstrcpy(funcItem[2]._itemName, TEXT("About"));
    funcItem[2]._pFunc = about;
    funcItem[2]._init2Check = false;
    funcItem[2]._pShKey = NULL;

    return funcItem;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification* notifyCode)
{
    if (!notifyCode) return;

    switch (notifyCode->nmhdr.code)
    {
    case NPPN_SHUTDOWN:
        commandMenuCleanUp();
        break;

    case NPPN_BUFFERACTIVATED:
    case NPPN_FILEOPENED:
        // Auto-apply if enabled and it's a markdown file
        if (g_stylesEnabled && isMarkdownFile()) {
            Sleep(100);
            applyMarkdownStyles();
        }
        break;

    case NPPN_FILESAVED:
        // Reapply after save if enabled
        if (g_stylesEnabled && isMarkdownFile()) {
            applyMarkdownStyles();
        }
        break;

    default:
        break;
    }
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif
