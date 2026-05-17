#pragma once

#include <gtk/gtk.h>

#include <string>
#include <vector>

#include "widget.hpp"

struct MenuItem {
    std::string label;
    std::function<void()> callback;
    std::string icon_name;
    bool sensitive = true;
};

class PopoverMenu : public Widget {
public:
    PopoverMenu()
        : Widget(gtk_popover_new())
    {}

    void add_child(Widget* child, const std::string& id = "") {
        gtk_popover_set_child(GTK_POPOVER(native), child->get_native());
    }

    void set_parent(Widget* parent) {
        gtk_widget_set_parent(native, parent->get_native());
    }

    void popup() {
        gtk_popover_popup(GTK_POPOVER(native));
    }

    void popdown() {
        gtk_popover_popdown(GTK_POPOVER(native));
    }

    void set_pointing_to(int x, int y, int w, int h) {
        GdkRectangle rect = {x, y, w, h};
        gtk_popover_set_pointing_to(GTK_POPOVER(native), &rect);
    }
};
