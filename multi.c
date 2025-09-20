#include "webview/webview.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Helper function to read a file into a string
char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, length, f);
    buffer[length] = '\0'; // null-terminate
    fclose(f);
    return buffer;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine,
                   int nCmdShow) {
  (void)hInst;
  (void)hPrevInst;
  (void)lpCmdLine;
  (void)nCmdShow;
#else
int main(void) {
#endif
    char* html = read_file("index.html");
    if (!html) {
        fprintf(stderr, "Failed to read index.html\n");
        return 1;
    }

    webview_t w = webview_create(0, NULL);
    webview_set_title(w, "Basic Example");
    webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);
    webview_set_html(w, html);
    webview_run(w);
    webview_destroy(w);

    free(html);
    return 0;
}
