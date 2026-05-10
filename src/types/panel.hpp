#pragma once

#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>

#include <string>
#include <vector>

#include "widget.hpp"

struct PanelProperties {
    std::string namespace_;

    Widget* child = nullptr;

    int monitor = 0;

    std::vector<std::string> anchor;

    bool exclusive = true;

    std::string layer = "top";

    std::string kb_mode = "none";

    bool popup = false;

    int height = 40;

    int width = -1;
};

class Panel : public Widget {
protected:
    GtkWidget* container;

public:
    Panel(const PanelProperties& properties)
        : Widget(gtk_window_new())
    {
        auto window =
            GTK_WINDOW(native);

        gtk_layer_init_for_window(
            window
        );

        gtk_layer_set_namespace(
            window,
            properties.namespace_.c_str()
        );

        // layer
        if (properties.layer == "top") {
            gtk_layer_set_layer(
                window,
                GTK_LAYER_SHELL_LAYER_TOP
            );
        }

        else if (
            properties.layer == "overlay"
        ) {
            gtk_layer_set_layer(
                window,
                GTK_LAYER_SHELL_LAYER_OVERLAY
            );
        }

        // anchors
        for (const auto& anchor :
            properties.anchor)
        {
            if (anchor == "top") {
                gtk_layer_set_anchor(
                    window,
                    GTK_LAYER_SHELL_EDGE_TOP,
                    true
                );
            }

            else if (anchor == "bottom") {
                gtk_layer_set_anchor(
                    window,
                    GTK_LAYER_SHELL_EDGE_BOTTOM,
                    true
                );
            }

            else if (anchor == "left") {
                gtk_layer_set_anchor(
                    window,
                    GTK_LAYER_SHELL_EDGE_LEFT,
                    true
                );
            }

            else if (anchor == "right") {
                gtk_layer_set_anchor(
                    window,
                    GTK_LAYER_SHELL_EDGE_RIGHT,
                    true
                );
            }
        }

        // exclusive zone
        if (properties.exclusive) {
            gtk_layer_auto_exclusive_zone_enable(
                window
            );
        }

        // keyboard mode
        if (
            properties.kb_mode == "none"
        ) {
            gtk_layer_set_keyboard_mode(
                window,
                GTK_LAYER_SHELL_KEYBOARD_MODE_NONE
            );
        }

        // child
        if (properties.child) {
            gtk_window_set_child(
                window,
                properties.child->get_native()
            );
        }

        // size
        gtk_widget_set_size_request(
            native,
            properties.width,
            properties.height
        );
    }

    void set_width(int width) {
        current_width = width;

        gtk_widget_set_size_request(
            native,
            current_width,
            current_height
        );
    }

    void set_height(int height) {
        current_height = height;

        gtk_widget_set_size_request(
            native,
            current_width,
            current_height
        );
    }

    void set_size(
        int width,
        int height
    ) {
        current_width = width;
        current_height = height;

        gtk_widget_set_size_request(
            native,
            current_width,
            current_height
        );
    }
    
    private:
        int current_width = -1;
        int current_height = -1;
};