#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

int main(int argc, char** argv) {
    gtk_init();

    GtkWidget* window = gtk_window_new();

    gtk_layer_init_for_window(GTK_WINDOW(window));

    gtk_layer_set_layer(
        GTK_WINDOW(window),
        GTK_LAYER_SHELL_LAYER_TOP
    );

    gtk_layer_set_anchor(
        GTK_WINDOW(window),
        GTK_LAYER_SHELL_EDGE_TOP,
        true
    );

    gtk_layer_set_anchor(
        GTK_WINDOW(window),
        GTK_LAYER_SHELL_EDGE_LEFT,
        true
    );

    gtk_layer_set_anchor(
        GTK_WINDOW(window),
        GTK_LAYER_SHELL_EDGE_RIGHT,
        true
    );

    gtk_window_set_decorated(
        GTK_WINDOW(window),
        false
    );

    gtk_widget_set_size_request(window, -1, 32);

    GtkWidget* label =
        gtk_label_new("hello");

    gtk_window_set_child(
        GTK_WINDOW(window),
        label
    );

    gtk_widget_show(window);

    GMainLoop* loop = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop);

    return 0;
}