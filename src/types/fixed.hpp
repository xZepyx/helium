#pragma once

#include <gtk/gtk.h>

#include "widget.hpp"

struct FixedChild {
    Widget* widget;
    int x;
    int y;
};

class Fixed : public Widget {
public:
    Fixed()
        : Widget(gtk_fixed_new())
    {}

    void put(Widget* child, int x, int y) {
        gtk_fixed_put(GTK_FIXED(native), child->get_native(), x, y);
    }

    void move(Widget* child, int x, int y) {
        gtk_fixed_move(GTK_FIXED(native), child->get_native(), x, y);
    }

    void remove(Widget* child) {
        gtk_fixed_remove(GTK_FIXED(native), child->get_native());
    }
};
