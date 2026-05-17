#pragma once

#include <gtk/gtk.h>

#include <functional>

#include "widget.hpp"

class Switch : public Widget {
private:
    std::function<void(bool)> _on_change;

    static void change_cb(GtkSwitch* sw, gboolean state, gpointer data) {
        auto self = static_cast<Switch*>(data);
        if (self && self->_on_change) self->_on_change(state);
    }

public:
    Switch(bool active = false)
        : Widget(gtk_switch_new())
    {
        gtk_switch_set_active(GTK_SWITCH(native), active);
    }

    void set_active(bool active) {
        gtk_switch_set_active(GTK_SWITCH(native), active);
    }

    bool get_active() {
        return gtk_switch_get_active(GTK_SWITCH(native));
    }

    void on_change(std::function<void(bool)> callback) {
        _on_change = callback;
        g_signal_connect(native, "state-set", G_CALLBACK(change_cb), this);
    }
};
