#pragma once
#include "widget.hpp"

class Entry : public Widget {
public:
    Entry() : Widget(gtk_entry_new()) {}

    void set_text(const std::string& text) {
        gtk_editable_set_text(GTK_EDITABLE(native), text.c_str());
    }

    std::string get_text() {
        return gtk_editable_get_text(GTK_EDITABLE(native));
    }
    
    // Triggered when user presses 'Enter'
    void on_activate(std::function<void()> callback) {
        // Similar g_signal_connect logic as Button
    }
};