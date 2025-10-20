#include <webkit2/webkit2.h>

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Create a GTK window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create a WebKit web view
    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new());

    // Load a page
    webkit_web_view_load_uri(webview, "https://example.org");

    // Put webview inside the window
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(webview));

    // Quit when window is closed
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
