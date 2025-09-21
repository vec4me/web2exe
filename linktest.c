#define UNICODE
#define _UNICODE

#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <string>

typedef HRESULT(__stdcall* PFN_CreateCoreWebView2EnvironmentWithOptions)(
    void* browserExecutableFolder,
    void* options,
    void* completionHandler
);

HWND g_hWnd = nullptr;
HMODULE g_hWebView2 = nullptr;

// Simple window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int wmain() {
    // --- Step 1: Load WebView2Loader.dll from executable directory ---
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash) *(lastSlash + 1) = 0; // keep directory
    wcscat_s(exePath, MAX_PATH, L"WebView2Loader.dll");

    g_hWebView2 = LoadLibraryW(exePath);
    if (!g_hWebView2) {
        wprintf(L"Failed to load WebView2Loader.dll from executable folder.\n");
        return 1;
    }

    PFN_CreateCoreWebView2EnvironmentWithOptions pCreateEnv =
        (PFN_CreateCoreWebView2EnvironmentWithOptions)GetProcAddress(
            g_hWebView2, "CreateCoreWebView2EnvironmentWithOptions"
        );

    if (!pCreateEnv) {
        wprintf(L"Failed to find CreateCoreWebView2EnvironmentWithOptions.\n");
        FreeLibrary(g_hWebView2);
        return 1;
    }

    wprintf(L"WebView2 DLL loaded dynamically!\n");

    // --- Step 2: Create a simple window ---
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"MyWebView2WindowClass";
    RegisterClassW(&wc);

    g_hWnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        L"Dynamic WebView2 Loader",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    if (!g_hWnd) {
        wprintf(L"Failed to create window.\n");
        FreeLibrary(g_hWebView2);
        return 1;
    }

    // --- Step 3: Normally here you would call pCreateEnv to create the WebView2 environment ---
    // Since we avoid including WebView2.h, you need to cast void* to COM interfaces manually
    // or use a small helper COM loader. For demonstration, we just show DLL loading + window.

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    FreeLibrary(g_hWebView2);
    return 0;
}
