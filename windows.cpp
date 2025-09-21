#define UNICODE
#define _UNICODE

#include <windows.h>
#include <wrl.h>
#include <wil/com.h>
#include <WebView2.h>

HWND g_hWnd = nullptr;
wil::com_ptr<ICoreWebView2Controller> g_webviewController;
wil::com_ptr<ICoreWebView2> g_webview;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_SIZE && g_webviewController) {
        RECT bounds;
        GetClientRect(hwnd, &bounds);
        g_webviewController->put_Bounds(bounds);
    } else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t WINDOW_NAME[] = L"WebView2Window";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_NAME;
    RegisterClass(&wc);

    g_hWnd = CreateWindowEx(
        0, WINDOW_NAME, WINDOW_NAME,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(g_hWnd, nCmdShow);

   
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
                                GetClientRect(g_hWnd, &bounds);
                                g_webviewController->put_Bounds(bounds);

                                g_webview->NavigateToString("<html></html>");
                            }
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
