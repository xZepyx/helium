#pragma once

#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

#include <string>
#include <vector>

#include "widget.hpp"
#include "../managers/windowmanager.hpp"

struct WindowProperties {
    std::string namespace_;
    int monitor = 0;
    std::vector<std::string> anchor;
    std::string exclusivity = "normal";
    std::string layer = "top";
    std::string kb_mode = "none";
    bool popup = false;
    int margin_top = 0;
    int margin_bottom = 0;
    int margin_left = 0;
    int margin_right = 0;
    bool dynamic_input_region = false;
};

class Window : public Widget {
private:
    std::string _namespace;
    std::vector<std::string> _anchor;
    std::string _exclusivity;
    std::string _layer;
    std::string _kb_mode;
    bool _popup;
    int _monitor;
    int _input_width = 0;
    int _input_height = 0;
    bool _dynamic_input_region;
    int _margin_top = 0, _margin_bottom = 0, _margin_left = 0, _margin_right = 0;

    void apply_anchors() {
        auto set = [&](const std::string& which, GtkLayerShellEdge edge) {
            for (const auto& a : _anchor) {
                if (a == which) {
                    gtk_layer_set_anchor(GTK_WINDOW(native), edge, TRUE);
                    return;
                }
            }
            gtk_layer_set_anchor(GTK_WINDOW(native), edge, FALSE);
        };
        set("top", GTK_LAYER_SHELL_EDGE_TOP);
        set("bottom", GTK_LAYER_SHELL_EDGE_BOTTOM);
        set("left", GTK_LAYER_SHELL_EDGE_LEFT);
        set("right", GTK_LAYER_SHELL_EDGE_RIGHT);
    }

    void change_input_region() {
        if (!_dynamic_input_region && _input_width == 0 && _input_height == 0) return;

        if (_input_width > 0 && _input_height > 0) {
            auto surface = gtk_native_get_surface(GTK_NATIVE(native));
            if (!surface) return;

            cairo_region_t* region = cairo_region_create();
            cairo_rectangle_int_t rect;
            rect.x = 0;
            rect.y = 0;
            rect.width = _input_width;
            rect.height = _input_height;
            cairo_region_union_rectangle(region, &rect);
            gdk_surface_set_input_region(surface, region);
            cairo_region_destroy(region);
        }
    }

public:
    Window(const WindowProperties& props)
        : Widget(gtk_window_new()),
          _namespace(props.namespace_),
          _anchor(props.anchor),
          _exclusivity(props.exclusivity),
          _layer(props.layer),
          _kb_mode(props.kb_mode),
          _popup(props.popup),
          _monitor(props.monitor),
          _dynamic_input_region(props.dynamic_input_region),
          _margin_top(props.margin_top),
          _margin_bottom(props.margin_bottom),
          _margin_left(props.margin_left),
          _margin_right(props.margin_right)
    {
        gtk_layer_init_for_window(GTK_WINDOW(native));
        gtk_layer_set_namespace(GTK_WINDOW(native), _namespace.c_str());
        set_monitor(_monitor);
        set_layer(_layer);
        set_kb_mode(_kb_mode);
        apply_anchors();
        set_exclusivity(_exclusivity);

        gtk_layer_set_margin(GTK_WINDOW(native), GTK_LAYER_SHELL_EDGE_TOP, _margin_top);
        gtk_layer_set_margin(GTK_WINDOW(native), GTK_LAYER_SHELL_EDGE_BOTTOM, _margin_bottom);
        gtk_layer_set_margin(GTK_WINDOW(native), GTK_LAYER_SHELL_EDGE_LEFT, _margin_left);
        gtk_layer_set_margin(GTK_WINDOW(native), GTK_LAYER_SHELL_EDGE_RIGHT, _margin_right);

        WindowManager::get_default().add_window(_namespace, this);
    }

    ~Window() {
        WindowManager::get_default().remove_window(_namespace);
    }

    void set_namespace(const std::string& ns) {
        _namespace = ns;
        gtk_layer_set_namespace(GTK_WINDOW(native), ns.c_str());
    }

    std::string get_namespace() { return _namespace; }

    void set_monitor(int mon) {
        _monitor = mon;
        auto display = gdk_display_get_default();
        if (!display) return;
        auto monitors = gdk_display_get_monitors(display);
        int n_monitors = g_list_model_get_n_items(G_LIST_MODEL(monitors));
        if (mon >= 0 && mon < n_monitors) {
            auto gdk_mon = GDK_MONITOR(g_list_model_get_item(G_LIST_MODEL(monitors), mon));
            gtk_layer_set_monitor(GTK_WINDOW(native), gdk_mon);
        }
    }

    int get_monitor() { return _monitor; }

    void set_layer(const std::string& layer) {
        _layer = layer;
        if (layer == "background") gtk_layer_set_layer(GTK_WINDOW(native), GTK_LAYER_SHELL_LAYER_BACKGROUND);
        else if (layer == "bottom") gtk_layer_set_layer(GTK_WINDOW(native), GTK_LAYER_SHELL_LAYER_BOTTOM);
        else if (layer == "top") gtk_layer_set_layer(GTK_WINDOW(native), GTK_LAYER_SHELL_LAYER_TOP);
        else if (layer == "overlay") gtk_layer_set_layer(GTK_WINDOW(native), GTK_LAYER_SHELL_LAYER_OVERLAY);
    }

    std::string get_layer() { return _layer; }

    void set_kb_mode(const std::string& mode) {
        _kb_mode = mode;
        if (mode == "none") gtk_layer_set_keyboard_mode(GTK_WINDOW(native), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);
        else if (mode == "exclusive") gtk_layer_set_keyboard_mode(GTK_WINDOW(native), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
        else if (mode == "on-demand") gtk_layer_set_keyboard_mode(GTK_WINDOW(native), GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);
    }

    std::string get_kb_mode() { return _kb_mode; }

    void set_exclusivity(const std::string& excl) {
        _exclusivity = excl;
        if (excl == "normal") {
            gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(native));
        } else if (excl == "none") {
            gtk_layer_set_exclusive_zone(GTK_WINDOW(native), 0);
        }
    }

    std::string get_exclusivity() { return _exclusivity; }

    void set_popup(bool pop) { _popup = pop; }
    bool get_popup() { return _popup; }

    void set_anchor(const std::vector<std::string>& anchor) {
        _anchor = anchor;
        apply_anchors();
    }

    std::vector<std::string> get_anchor() { return _anchor; }

    void set_margin(const std::string& side, int value) {
        GtkLayerShellEdge edge;
        if (side == "top") { _margin_top = value; edge = GTK_LAYER_SHELL_EDGE_TOP; }
        else if (side == "bottom") { _margin_bottom = value; edge = GTK_LAYER_SHELL_EDGE_BOTTOM; }
        else if (side == "left") { _margin_left = value; edge = GTK_LAYER_SHELL_EDGE_LEFT; }
        else { _margin_right = value; edge = GTK_LAYER_SHELL_EDGE_RIGHT; }
        gtk_layer_set_margin(GTK_WINDOW(native), edge, value);
    }

    void set_input_width(int w) { _input_width = w; change_input_region(); }
    int get_input_width() { return _input_width; }

    void set_input_height(int h) { _input_height = h; change_input_region(); }
    int get_input_height() { return _input_height; }

    void set_dynamic_input_region(bool v) { _dynamic_input_region = v; }
    bool get_dynamic_input_region() { return _dynamic_input_region; }

    void set_child(Widget* child) override {
        gtk_window_set_child(GTK_WINDOW(native), child->get_native());
    }
};
