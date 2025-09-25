#define UNICODE
#define _UNICODE
#include <windows.h>
#include <WebView2.h>

ICoreWebView2Environment* g_env = NULL;
ICoreWebView2Controller* g_controller = NULL;
ICoreWebView2* g_webview = NULL;
HWND g_hwnd = NULL;

typedef struct ControllerHandler ControllerHandler;
typedef struct {
    HRESULT(STDMETHODCALLTYPE* QueryInterface)(ControllerHandler*, REFIID, void**);
    ULONG(STDMETHODCALLTYPE* AddRef)(ControllerHandler*);
    ULONG(STDMETHODCALLTYPE* Release)(ControllerHandler*);
    HRESULT(STDMETHODCALLTYPE* Invoke)(ControllerHandler*, HRESULT, ICoreWebView2Controller*);
} ControllerHandlerVtbl;

struct ControllerHandler {
    ControllerHandlerVtbl* lpVtbl;
    ULONG ref;
};

ULONG STDMETHODCALLTYPE Controller_AddRef(ControllerHandler* This) { return ++This->ref; }
ULONG STDMETHODCALLTYPE Controller_Release(ControllerHandler* This) { return --This->ref; }
HRESULT STDMETHODCALLTYPE Controller_QueryInterface(ControllerHandler* This, REFIID riid, void** ppvObject) {
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE Controller_Invoke(ControllerHandler* This, HRESULT result, ICoreWebView2Controller* controller) {
    if (FAILED(result) || !controller) return result;
    g_controller = controller;
    g_controller->lpVtbl->AddRef(g_controller);

    g_controller->lpVtbl->get_CoreWebView2(g_controller, &g_webview);
    if (g_webview) {
        g_controller->lpVtbl->put_IsVisible(g_controller, TRUE);
        RECT bounds;
        GetClientRect(g_hwnd, &bounds);
        g_controller->lpVtbl->put_Bounds(g_controller, bounds);
        g_webview->lpVtbl->NavigateToString(g_webview, L"<html></html>");
    }
    return S_OK;
}

ControllerHandlerVtbl g_controllerVTable = { Controller_QueryInterface, Controller_AddRef, Controller_Release, Controller_Invoke };
static ControllerHandler g_controllerHandler = { &g_controllerVTable, 1 };

typedef struct EnvHandler EnvHandler;
typedef struct {
    HRESULT(STDMETHODCALLTYPE* QueryInterface)(EnvHandler*, REFIID, void**);
    ULONG(STDMETHODCALLTYPE* AddRef)(EnvHandler*);
    ULONG(STDMETHODCALLTYPE* Release)(EnvHandler*);
    HRESULT(STDMETHODCALLTYPE* Invoke)(EnvHandler*, HRESULT, ICoreWebView2Environment*);
} EnvHandlerVtbl;

struct EnvHandler {
    EnvHandlerVtbl* lpVtbl;
    ULONG ref;
};

ULONG STDMETHODCALLTYPE Env_AddRef(EnvHandler* This) { return ++This->ref; }
ULONG STDMETHODCALLTYPE Env_Release(EnvHandler* This) { return --This->ref; }
HRESULT STDMETHODCALLTYPE Env_QueryInterface(EnvHandler* This, REFIID riid, void** ppvObject) {
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE Env_Invoke(EnvHandler* This, HRESULT result, ICoreWebView2Environment* env) {
    if (FAILED(result) || !env) return result;
    g_env = env;
    g_env->lpVtbl->AddRef(g_env);

    g_env->lpVtbl->CreateCoreWebView2Controller(
        g_env,
        g_hwnd,
        (ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*)&g_controllerHandler
    );
    return S_OK;
}

EnvHandlerVtbl g_envHandlerVTable = { Env_QueryInterface, Env_AddRef, Env_Release, Env_Invoke };
static EnvHandler g_envHandler = { &g_envHandlerVTable, 1 };

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_SIZE && g_controller) {
        RECT bounds;
        GetClientRect(hwnd, &bounds);
        g_controller->lpVtbl->put_Bounds(g_controller, bounds);
    } else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

__declspec(noreturn) void __stdcall WinMainCRTStartup(void) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    const wchar_t CLASS_NAME[] = L"WebView2Window";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    g_hwnd = CreateWindowEx(
        0, CLASS_NAME, L"WebView2 Minimal C",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);

    CreateCoreWebView2EnvironmentWithOptions(
        NULL, NULL, NULL,
        (ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*)&g_envHandler
    );

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ExitProcess(0);
}
