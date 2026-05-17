#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <string>

#include "widget.hpp"

class ToggleButton : public Widget {
private:
    std::function<void()> _on_toggled;

    static void toggled_cb(GtkToggleButton*, gpointer data) {
        auto self = static_cast<ToggleButton*>(data);
        if (self && self->_on_toggled) self->_on_toggled();
    }

public:
    ToggleButton(const std::string& label = "", bool active = false)
        : Widget(gtk_toggle_button_new_with_label(label.c_str()))
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(native), active);
    }

    ToggleButton(Widget* child, bool active = false)
        : Widget(gtk_toggle_button_new())
    {
        gtk_button_set_child(GTK_BUTTON(native), child->get_native());
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(native), active);
    }

    void set_active(bool active) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(native), active);
    }

    bool get_active() {
        return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(native));
    }

    void set_label(const std::string& label) {
        gtk_button_set_label(GTK_BUTTON(native), label.c_str());
    }

    std::string get_label() {
        return gtk_button_get_label(GTK_BUTTON(native));
    }

    void on_toggled(std::function<void()> callback) {
        _on_toggled = callback;
        g_signal_connect(native, "toggled", G_CALLBACK(toggled_cb), this);
    }

    void on_click(std::function<void()> callback) {
        connect_signal("clicked", callback);
    }
};
