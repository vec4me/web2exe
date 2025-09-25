#define UNICODE
#define _UNICODE

#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>

HWND g_hWnd = nullptr;
wil::com_ptr<ICoreWebView2Controller> g_webviewController;
wil::com_ptr<ICoreWebView2> g_webview;

// Typedefs for User32 functions
typedef HWND(WINAPI* pCreateWindowExW)(DWORD, LPCWSTR, LPCWSTR, DWORD,
                                       int, int, int, int,
                                       HWND, HMENU, HINSTANCE, LPVOID);
typedef ATOM(WINAPI* pRegisterClassW)(const WNDCLASSW*);
typedef BOOL(WINAPI* pShowWindow)(HWND, int);
typedef BOOL(WINAPI* pGetClientRect)(HWND, LPRECT);
typedef LRESULT(CALLBACK* pDefWindowProcW)(HWND, UINT, WPARAM, LPARAM);
typedef BOOL(WINAPI* pTranslateMessage)(const MSG*);
typedef LRESULT(WINAPI* pDispatchMessageW)(const MSG*);
typedef BOOL(WINAPI* pGetMessageW)(MSG*, HWND, UINT, UINT);
typedef void(WINAPI* pPostQuitMessage)(int);

// Function pointers
pCreateWindowExW CreateWindowExW_ = nullptr;
pRegisterClassW RegisterClassW_ = nullptr;
pShowWindow ShowWindow_ = nullptr;
pGetClientRect GetClientRect_ = nullptr;
pDefWindowProcW DefWindowProcW_ = nullptr;
pTranslateMessage TranslateMessage_ = nullptr;
pDispatchMessageW DispatchMessage_ = nullptr;
pGetMessageW GetMessage_ = nullptr;
pPostQuitMessage PostQuitMessage_ = nullptr;

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_SIZE && g_webviewController) {
        RECT bounds;
        GetClientRect_(hwnd, &bounds);
        g_webviewController->put_Bounds(bounds);
    } else if (msg == WM_DESTROY) {
        PostQuitMessage_(0);
        return 0;
    }
    return DefWindowProcW_(hwnd, msg, wParam, lParam);
}

int main() {
    // Dynamically load User32.dll
    HMODULE hUser32 = LoadLibraryW(L"user32.dll");
    if (!hUser32) return -1;

    CreateWindowExW_ = (pCreateWindowExW)GetProcAddress(hUser32, "CreateWindowExW");
    RegisterClassW_ = (pRegisterClassW)GetProcAddress(hUser32, "RegisterClassW");
    ShowWindow_ = (pShowWindow)GetProcAddress(hUser32, "ShowWindow");
    GetClientRect_ = (pGetClientRect)GetProcAddress(hUser32, "GetClientRect");
    DefWindowProcW_ = (pDefWindowProcW)GetProcAddress(hUser32, "DefWindowProcW");
    TranslateMessage_ = (pTranslateMessage)GetProcAddress(hUser32, "TranslateMessage");
    DispatchMessage_ = (pDispatchMessageW)GetProcAddress(hUser32, "DispatchMessageW");
    GetMessage_ = (pGetMessageW)GetProcAddress(hUser32, "GetMessageW");
    PostQuitMessage_ = (pPostQuitMessage)GetProcAddress(hUser32, "PostQuitMessage");

    // Setup window
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    const wchar_t WINDOW_NAME[] = L"WebView2Window";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_NAME;
    RegisterClassW_(&wc);

    g_hWnd = CreateWindowExW_(0, WINDOW_NAME, WINDOW_NAME,
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              nullptr, nullptr, hInstance, nullptr);

    ShowWindow_(g_hWnd, SW_SHOW);

    // WebView2 initialization (statically linked)
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT, ICoreWebView2Environment* env) -> HRESULT {
                if (!env) return E_FAIL;
                env->CreateCoreWebView2Controller(
                    g_hWnd,
                    Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT, ICoreWebView2Controller* controller) -> HRESULT {
                            if (controller) {
                                g_webviewController = controller;
                                g_webviewController->get_CoreWebView2(&g_webview);

                                RECT bounds;
                                GetClientRect_(g_hWnd, &bounds);
                                g_webviewController->put_Bounds(bounds);

                                g_webview->NavigateToString("<html></html>");
                            }
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());

    // Message loop
    MSG msg{};
    while (GetMessage_(&msg, nullptr, 0, 0)) {
        TranslateMessage_(&msg);
        DispatchMessage_(&msg);
    }

    return 0;
}
