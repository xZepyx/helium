#pragma once
#include <gtk/gtk.h>
#include "widget.hpp"

class Overlay : public Widget {
public:
    Overlay() : Widget(gtk_overlay_new()) {}

    void set_child(Widget* child) override {
        gtk_overlay_set_child(GTK_OVERLAY(native), child->get_native());
    }

    void add_overlay(Widget* widget) {
        gtk_overlay_add_overlay(GTK_OVERLAY(native), widget->get_native());
    }
};