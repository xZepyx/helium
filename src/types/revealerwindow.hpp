#pragma once

#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

#include <string>
#include <vector>

#include "widget.hpp"
#include "../managers/windowmanager.hpp"

class RevealerWindow : public Widget {
private:
    std::string _namespace;
    std::vector<std::string> _anchor;
    int _monitor;
    bool _visible = false;

    static void on_destroy(GtkWidget*, gpointer data) {
        auto self = static_cast<RevealerWindow*>(data);
        if (self) self->_visible = false;
    }

public:
    RevealerWindow(const std::string& namespace_, int monitor = 0,
                   const std::vector<std::string>& anchor = {},
                   const std::string& layer = "overlay")
        : Widget(gtk_window_new()),
          _namespace(namespace_), _anchor(anchor), _monitor(monitor)
    {
        gtk_layer_init_for_window(GTK_WINDOW(native));
        gtk_layer_set_namespace(GTK_WINDOW(native), namespace_.c_str());
        gtk_layer_set_layer(GTK_WINDOW(native), GTK_LAYER_SHELL_LAYER_OVERLAY);
        gtk_layer_set_keyboard_mode(GTK_WINDOW(native), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);

        for (const auto& a : anchor) {
            if (a == "top") gtk_layer_set_anchor(GTK_WINDOW(native), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
            else if (a == "bottom") gtk_layer_set_anchor(GTK_WINDOW(native), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
            else if (a == "left") gtk_layer_set_anchor(GTK_WINDOW(native), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
            else if (a == "right") gtk_layer_set_anchor(GTK_WINDOW(native), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
        }

        if (monitor >= 0) {
            auto display = gdk_display_get_default();
            if (display) {
                auto monitors = gdk_display_get_monitors(display);
                int n = g_list_model_get_n_items(monitors);
                if (monitor < n) {
                    auto gdk_mon = GDK_MONITOR(g_list_model_get_item(monitors, monitor));
                    gtk_layer_set_monitor(GTK_WINDOW(native), gdk_mon);
                }
            }
        }

        g_signal_connect(native, "destroy", G_CALLBACK(on_destroy), this);

        WindowManager::get_default().add_window(namespace_, this);
    }

    ~RevealerWindow() {
        WindowManager::get_default().remove_window(_namespace);
    }

    void set_child(Widget* child) override {
        gtk_window_set_child(GTK_WINDOW(native), child->get_native());
    }

    void reveal() {
        _visible = true;
        gtk_widget_set_visible(native, TRUE);
    }

    void hide_window() {
        _visible = false;
        gtk_widget_set_visible(native, FALSE);
    }

    void toggle() {
        if (_visible) hide_window();
        else reveal();
    }

    bool is_visible() { return _visible; }
    std::string get_namespace() { return _namespace; }
};
