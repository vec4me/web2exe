#define UNICODE
#define _UNICODE
#include <windows.h>
#include <WebView2.h>
#include <wchar.h>

// ---------------- Global pointers ----------------
ICoreWebView2Environment* g_env = NULL;
ICoreWebView2Controller* g_controller = NULL;
ICoreWebView2* g_webview = NULL;
HWND g_hwnd = NULL;

// ---------------- Controller Handler ----------------
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

ULONG STDMETHODCALLTYPE Controller_AddRef(ControllerHandler* This) {
    return ++This->ref;
}

ULONG STDMETHODCALLTYPE Controller_Release(ControllerHandler* This) {
    return --This->ref;
}

HRESULT STDMETHODCALLTYPE Controller_QueryInterface(ControllerHandler* This, REFIID riid, void** ppvObject) {
    *ppvObject = This;
    Controller_AddRef(This);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE Controller_Invoke(ControllerHandler* This, HRESULT result, ICoreWebView2Controller* controller) {
    g_controller = controller;
    controller->get_CoreWebView2(&g_webview);
    controller->put_IsVisible(TRUE);

    RECT bounds;
    GetClientRect(g_hwnd, &bounds);
    controller->put_Bounds(bounds);

    g_webview->NavigateToString(L"<html><body><h1>Hello WebView2!</h1></body></html>");
    return S_OK;
}

ControllerHandlerVtbl g_controllerVTable = {
    Controller_QueryInterface,
    Controller_AddRef,
    Controller_Release,
    Controller_Invoke
};

static ControllerHandler g_controllerHandler = { &g_controllerVTable, 1 };

// ---------------- Environment Handler ----------------
typedef struct EnvironmentHandler EnvironmentHandler;
typedef struct {
    HRESULT(STDMETHODCALLTYPE* QueryInterface)(EnvironmentHandler*, REFIID, void**);
    ULONG(STDMETHODCALLTYPE* AddRef)(EnvironmentHandler*);
    ULONG(STDMETHODCALLTYPE* Release)(EnvironmentHandler*);
    HRESULT(STDMETHODCALLTYPE* Invoke)(EnvironmentHandler*, HRESULT, ICoreWebView2Environment*);
} EnvironmentHandlerVtbl;

struct EnvironmentHandler {
    EnvironmentHandlerVtbl* lpVtbl;
    ULONG ref;
};

ULONG STDMETHODCALLTYPE Env_AddRef(EnvironmentHandler* This) {
    return ++This->ref;
}

ULONG STDMETHODCALLTYPE Env_Release(EnvironmentHandler* This) {
    return --This->ref;
}

HRESULT STDMETHODCALLTYPE Env_QueryInterface(EnvironmentHandler* This, REFIID riid, void** ppvObject) {
    *ppvObject = This;
    Env_AddRef(This);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE Env_Invoke(EnvironmentHandler* This, HRESULT result, ICoreWebView2Environment* env) {
    g_env = env;
    g_env->CreateCoreWebView2Controller(
        g_hwnd,
        (ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*)&g_controllerHandler
    );
    return S_OK;
}

EnvironmentHandlerVtbl g_envHandlerVTable = {
    Env_QueryInterface,
    Env_AddRef,
    Env_Release,
    Env_Invoke
};

static EnvironmentHandler g_envHandler = { &g_envHandlerVTable, 1 };

// ---------------- Window Procedure ----------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_SIZE && g_controller) {
        RECT bounds;
        GetClientRect(hwnd, &bounds);
        g_controller->put_Bounds(bounds);
    }
    else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ---------------- WinMain ----------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    const wchar_t CLASS_NAME[] = L"WebView2Window";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    g_hwnd = CreateWindowEx(0, CLASS_NAME, L"WebView2 C Example",
                            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                            NULL, NULL, hInstance, NULL);

    ShowWindow(g_hwnd, nCmdShow);

    CreateCoreWebView2EnvironmentWithOptions(
        NULL, NULL, NULL,
        (ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*)&g_envHandler
    );

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
