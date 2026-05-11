#pragma once

#include <gtk/gtk.h>
#include <functional>

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

    void connect_signal(const std::string& signal_name, std::function<void()> callback) {
            // We heap-allocate the function so it persists until the signal is disconnected
            auto* callback_ptr = new std::function<void()>(callback);

            g_signal_connect_data(
                native,
                signal_name.c_str(),
                G_CALLBACK(+[](GtkWidget*, gpointer data) {
                    auto* cb = static_cast<std::function<void()>*>(data);
                    if (cb && *cb) (*cb)();
                }),
                callback_ptr,
                [](gpointer data, GClosure*) {
                    delete static_cast<std::function<void()>*>(data);
                },
                (GConnectFlags)0
            );
        }

    virtual void set_child(Widget* child) {} // setchild method
};