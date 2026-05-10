#pragma once

#include <gtk/gtk.h>

class Widget {
protected:
    GtkWidget* native;

public:
    Widget(GtkWidget* widget)
        : native(widget) {}

    virtual ~Widget() = default;

    GtkWidget* get_native() {
        return native;
    }

    void show() {
        gtk_widget_show(native);
    }

    void add_css_class(const std::string& class_name) {
        gtk_widget_add_css_class(
            native,
            class_name.c_str()
        );
    }

    void remove_css_class(const std::string& class_name) {
        gtk_widget_remove_css_class(
            native,
            class_name.c_str()
        );
    }

    virtual void set_child(Widget* child) {} // setchild method
};