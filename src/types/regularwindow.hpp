#pragma once

#include <gtk/gtk.h>

#include <string>

#include "widget.hpp"
#include "../managers/windowmanager.hpp"

class RegularWindow : public Widget {
private:
    std::string _namespace;
    std::string _title;

public:
    RegularWindow(const std::string& namespace_, const std::string& title = "")
        : Widget(gtk_window_new()), _namespace(namespace_), _title(title)
    {
        gtk_window_set_title(GTK_WINDOW(native), title.c_str());
        WindowManager::get_default().add_window(namespace_, this);
    }

    ~RegularWindow() {
        WindowManager::get_default().remove_window(_namespace);
    }

    void set_title(const std::string& t) {
        _title = t;
        gtk_window_set_title(GTK_WINDOW(native), t.c_str());
    }

    std::string get_title() { return _title; }
    std::string get_namespace() { return _namespace; }

    void set_child(Widget* child) override {
        gtk_window_set_child(GTK_WINDOW(native), child->get_native());
    }

    void set_titlebar(Widget* tb) {
        gtk_window_set_titlebar(GTK_WINDOW(native), tb->get_native());
    }

    void set_default_size(int w, int h) {
        gtk_window_set_default_size(GTK_WINDOW(native), w, h);
    }
};
