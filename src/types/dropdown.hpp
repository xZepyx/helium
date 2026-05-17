#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <string>
#include <vector>

#include "widget.hpp"

class DropDown : public Widget {
private:
    std::vector<std::string> _items;
    GtkStringList* _model;
    std::function<void(int)> _on_selected;

    static void selected_cb(GtkDropDown* dropdown, GParamSpec*, gpointer data) {
        auto self = static_cast<DropDown*>(data);
        if (!self || !self->_on_selected) return;
        int pos = gtk_drop_down_get_selected(dropdown);
        self->_on_selected(pos);
    }

public:
    DropDown(const std::vector<std::string>& items = {}, int selected = 0)
        : Widget(gtk_drop_down_new(nullptr, nullptr)),
          _items(items)
    {
        _model = gtk_string_list_new(nullptr);
        for (const auto& item : items) {
            gtk_string_list_append(_model, item.c_str());
        }
        gtk_drop_down_set_model(GTK_DROP_DOWN(native), G_LIST_MODEL(_model));
        gtk_drop_down_set_selected(GTK_DROP_DOWN(native), selected);

        g_signal_connect(native, "notify::selected", G_CALLBACK(selected_cb), this);
    }

    void set_items(const std::vector<std::string>& items) {
        _items = items;
        _model = gtk_string_list_new(nullptr);
        for (const auto& item : items) {
            gtk_string_list_append(_model, item.c_str());
        }
        gtk_drop_down_set_model(GTK_DROP_DOWN(native), G_LIST_MODEL(_model));
    }

    std::vector<std::string> get_items() { return _items; }

    void set_selected(int index) {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(native), index);
    }

    int get_selected() {
        return gtk_drop_down_get_selected(GTK_DROP_DOWN(native));
    }

    std::string get_selected_item() {
        auto pos = gtk_drop_down_get_selected(GTK_DROP_DOWN(native));
        if (pos < 0 || pos >= (int)_items.size()) return "";
        return _items[pos];
    }

    void on_selected(std::function<void(int)> callback) {
        _on_selected = callback;
    }
};
