#pragma once

#include <gtk/gtk.h>

#include <functional>

#include "widget.hpp"

class ListBoxRow : public Widget {
private:
    bool _selected = false;
    std::function<void()> _on_activate;

    static void activate_cb(GtkListBoxRow*, gpointer data) {
        auto self = static_cast<ListBoxRow*>(data);
        if (self && self->_on_activate) self->_on_activate();
    }

public:
    ListBoxRow()
        : Widget(gtk_list_box_row_new())
    {
        g_signal_connect(native, "activate", G_CALLBACK(activate_cb), this);
    }

    void set_child(Widget* child) override {
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(native), child->get_native());
    }

    void set_selected(bool selected) {
        _selected = selected;
        if (selected) add_css_class("selected");
        else remove_css_class("selected");
    }

    bool is_selected() { return _selected; }

    void on_activate(std::function<void()> callback) {
        _on_activate = callback;
    }

    void activate() {
        if (_on_activate) _on_activate();
    }
};
