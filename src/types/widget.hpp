#pragma once

#include <gtk/gtk.h>
#include <functional>
#include <string>

class Widget {
protected:
    GtkWidget* native;
    GtkCssProvider* inline_provider = nullptr;

public:
    Widget(GtkWidget* widget)
        : native(widget) {
        if (native) g_object_ref_sink(native);
    }

    virtual ~Widget() {
        if (native) g_object_unref(native);
    }

    GtkWidget* get_native() {
        return native;
    }

    void show() {
        gtk_widget_set_visible(native, TRUE);
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

    void set_style(const std::string& css_style) {
        if (inline_provider != nullptr) {
            gtk_style_context_remove_provider_for_display(
                gdk_display_get_default(),
                GTK_STYLE_PROVIDER(inline_provider)
            );
            g_object_unref(inline_provider);
        }

        inline_provider = gtk_css_provider_new();
        
        // 1. Give this specific widget a unique ID based on its pointer address
        std::string widget_id = "widget_" + std::to_string(reinterpret_cast<uintptr_t>(native));
        gtk_widget_set_name(native, widget_id.c_str());

        // 2. Target ONLY this ID in the CSS string
        // This prevents the font-family from leaking to other widgets
        std::string wrapped_css = "#" + widget_id + " { " + css_style + " }";
        
        gtk_css_provider_load_from_data(inline_provider, wrapped_css.c_str(), -1);

        gtk_style_context_add_provider_for_display(
            gdk_display_get_default(),
            GTK_STYLE_PROVIDER(inline_provider),
            GTK_STYLE_PROVIDER_PRIORITY_USER
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

protected:
    void set_native(GtkWidget* widget) {
        if (native) g_object_unref(native);
        native = widget;
        if (native) g_object_ref_sink(native);
    }
};