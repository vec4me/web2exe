#define UNICODE
#include <Windows.h>
#include <WebView2.h>
#include <stdint.h>
#include <string.h>

ICoreWebView2Environment* g_env = NULL;
ICoreWebView2Controller* g_controller = NULL;
ICoreWebView2* g_webview = NULL;
HWND g_hwnd = NULL;

static const char html_data[] = {/*HTML_DATA_PLACEHOLDER*/};
static const size_t html_data_len = /*HTML_LEN_PLACEHOLDER*/;

static const unsigned char webview2_dll[] = {/*DLL_DATA_PLACEHOLDER*/};
static const unsigned int webview2_dll_len = /*DLL_LEN_PLACEHOLDER*/;

typedef struct Handler Handler;

typedef struct {
    HRESULT(STDMETHODCALLTYPE* QueryInterface)(Handler*, REFIID, void**);
    ULONG(STDMETHODCALLTYPE* AddRef)(Handler*);
    ULONG(STDMETHODCALLTYPE* Release)(Handler*);
    HRESULT(STDMETHODCALLTYPE* Invoke)(Handler*, HRESULT, void*);
} HandlerVtbl;

struct Handler {
    HandlerVtbl* lpVtbl;
    ULONG ref;
};

ULONG STDMETHODCALLTYPE Handler_AddRef(Handler* This) { return 1; }
ULONG STDMETHODCALLTYPE Handler_Release(Handler* This) { return 1; }
HRESULT STDMETHODCALLTYPE Handler_QueryInterface(Handler* This, REFIID riid, void** ppvObject) {
    *ppvObject = NULL; return E_NOINTERFACE;
}

typedef HRESULT(STDMETHODCALLTYPE* PFN_CreateCoreWebView2EnvironmentWithOptions)(
    LPCWSTR, LPCWSTR, ICoreWebView2EnvironmentOptions*, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);
static PFN_CreateCoreWebView2EnvironmentWithOptions pCreateEnvironment = NULL;

static HMODULE load_dll_from_memory(const unsigned char* data, size_t len) {
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    lstrcatW(tmp, L"WebView2Loader.dll");

    HANDLE f = CreateFileW(tmp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) return NULL;

    DWORD written;
    WriteFile(f, data, (DWORD)len, &written, NULL);
    CloseHandle(f);

    HMODULE mod = LoadLibraryW(tmp);
    DeleteFileW(tmp);
    return mod;
}

static int load_webview2loader_from_memory(void) {
    HMODULE mod = load_dll_from_memory(webview2_dll, webview2_dll_len);
    if (!mod) return 0;
    pCreateEnvironment = (PFN_CreateCoreWebView2EnvironmentWithOptions)
        GetProcAddress(mod, "CreateCoreWebView2EnvironmentWithOptions");
    return pCreateEnvironment != NULL;
}


HRESULT STDMETHODCALLTYPE Controller_Invoke(Handler* h, HRESULT res, void* obj) {
    if (FAILED(res) || !obj) return res;

    g_controller = obj;
    g_controller->lpVtbl->AddRef(g_controller);
    g_controller->lpVtbl->get_CoreWebView2(g_controller, &g_webview);
    g_controller->lpVtbl->put_IsVisible(g_controller, TRUE);

    RECT r; GetClientRect(g_hwnd, &r);
    g_controller->lpVtbl->put_Bounds(g_controller, r);

    static wchar_t html_w[/*HTML_LEN_PLACEHOLDER*/ + 1];
    MultiByteToWideChar(CP_UTF8, 0, html_data, (int)html_data_len, html_w, sizeof(html_w)/sizeof(wchar_t));
    html_w[html_data_len] = 0;
    g_webview->lpVtbl->NavigateToString(g_webview, html_w);

    return S_OK;
}
Handler g_controllerHandler = { &(HandlerVtbl){ Handler_QueryInterface, Handler_AddRef, Handler_Release, Controller_Invoke }, 1 };


HRESULT STDMETHODCALLTYPE Env_Invoke(Handler* h, HRESULT res, void* obj) {
    if (FAILED(res) || !obj) return res;

    g_env = obj;
    g_env->lpVtbl->AddRef(g_env);

    g_env->lpVtbl->CreateCoreWebView2Controller(
        g_env, g_hwnd, (ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*)&g_controllerHandler
    );

    return S_OK;
}
Handler g_envHandler = { &(HandlerVtbl){ Handler_QueryInterface, Handler_AddRef, Handler_Release, Env_Invoke }, 1 };

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    if (msg == WM_SIZE && g_controller) {
        RECT r; GetClientRect(hwnd, &r);
        g_controller->lpVtbl->put_Bounds(g_controller, r);
    } else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, w, l);
}

__declspec(noreturn) void WinMainCRTStartup(void) {
    HINSTANCE hInst = GetModuleHandle(NULL);
    const wchar_t window_name[] = L"/*WINDOW_NAME*/";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = window_name;
    RegisterClass(&wc);

    g_hwnd = CreateWindowEx(0, window_name, window_name,
                            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                            NULL, NULL, hInst, NULL);
    ShowWindow(g_hwnd, SW_SHOWDEFAULT);

    if (!load_webview2loader_from_memory()) ExitProcess(1);

    wchar_t tmpFolder[MAX_PATH];
    GetTempPathW(MAX_PATH, tmpFolder);

    pCreateEnvironment(NULL, tmpFolder, NULL,
        (ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*)&g_envHandler
    );

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
