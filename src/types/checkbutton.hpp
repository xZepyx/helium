#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <string>

#include "widget.hpp"

class CheckButton : public Widget {
private:
    std::function<void()> _on_toggled;

    static void toggled_cb(GtkToggleButton*, gpointer data) {
        auto self = static_cast<CheckButton*>(data);
        if (self && self->_on_toggled) self->_on_toggled();
    }

public:
    CheckButton(const std::string& label = "", bool active = false)
        : Widget(gtk_check_button_new_with_label(label.c_str()))
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(native), active);
        add_css_class("check-button");
    }

    void set_active(bool active) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(native), active);
    }

    bool get_active() {
        return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(native));
    }

    void set_label(const std::string& label) {
        gtk_check_button_set_label(GTK_CHECK_BUTTON(native), label.c_str());
    }

    std::string get_label() {
        return gtk_check_button_get_label(GTK_CHECK_BUTTON(native));
    }

    void set_group(CheckButton* group) {
        gtk_check_button_set_group(GTK_CHECK_BUTTON(native), GTK_CHECK_BUTTON(group->get_native()));
    }

    void on_toggled(std::function<void()> callback) {
        _on_toggled = callback;
        g_signal_connect(native, "toggled", G_CALLBACK(toggled_cb), this);
    }
};
