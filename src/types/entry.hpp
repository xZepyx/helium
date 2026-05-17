#pragma once
#include <gtk/gtk.h>
#include <functional>
#include <string>
#include "widget.hpp"

class Entry : public Widget {
private:
    gulong _activate_handler = 0;
    gulong _change_handler = 0;
    std::function<void()> _on_activate;
    std::function<void()> _on_change;

    static void activate_cb(GtkEntry*, gpointer data) {
        auto self = static_cast<Entry*>(data);
        if (self && self->_on_activate) self->_on_activate();
    }

    static void change_cb(GtkEditable*, GParamSpec*, gpointer data) {
        auto self = static_cast<Entry*>(data);
        if (self && self->_on_change) self->_on_change();
    }

public:
    Entry() : Widget(gtk_entry_new()) {}

    void set_text(const std::string& text) {
        gtk_editable_set_text(GTK_EDITABLE(native), text.c_str());
    }

    std::string get_text() {
        return gtk_editable_get_text(GTK_EDITABLE(native));
    }
    
    void on_activate(std::function<void()> callback) {
        _on_activate = callback;
        if (_activate_handler) g_signal_handler_disconnect(native, _activate_handler);
        _activate_handler = g_signal_connect(native, "activate", G_CALLBACK(activate_cb), this);
    }

    void on_change(std::function<void()> callback) {
        _on_change = callback;
        if (_change_handler) g_signal_handler_disconnect(native, _change_handler);
        _change_handler = g_signal_connect(native, "notify::text", G_CALLBACK(change_cb), this);
    }
};