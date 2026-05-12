#pragma once
#include <gtk/gtk.h>
#include "widget.hpp"

class ScrolledWindow : public Widget {
public:
    ScrolledWindow() : Widget(gtk_scrolled_window_new()) {
        // Default to automatic scrollbars
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(native), 
                                     GTK_POLICY_AUTOMATIC, 
                                     GTK_POLICY_AUTOMATIC);
    }

    void set_child(Widget* child) override {
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(native), child->get_native());
    }
    
    void set_policy(std::string h, std::string v) {
        auto parse = [](std::string s) {
            if (s == "always") return GTK_POLICY_ALWAYS;
            if (s == "never") return GTK_POLICY_NEVER;
            return GTK_POLICY_AUTOMATIC;
        };
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(native), parse(h), parse(v));
    }
};