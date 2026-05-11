#pragma once

#include <gtk/gtk.h>

#include <string>

#include "widget.hpp"

struct ButtonProperties {
    std::string label;
};

class Button : public Widget {
public:
    Button(const ButtonProperties& props)
        : Widget(gtk_button_new_with_label(props.label.c_str()))
    {}

    // Connect a C++ lambda or function to the "clicked" signal
    void on_click(std::function<void()> callback) {
        connect_signal("clicked", callback);
    }
};