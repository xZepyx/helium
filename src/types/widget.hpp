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

    void set_style(const std::string& css_style) {
        GtkStyleContext* context = gtk_widget_get_style_context(native);

        // If we already have an inline provider, remove it first to update so it won't fuck up
        if (inline_provider != nullptr) {
            gtk_style_context_remove_provider(context, GTK_STYLE_PROVIDER(inline_provider));
            g_object_unref(inline_provider);
        }

        inline_provider = gtk_css_provider_new();
        
        // We wrap the user's string in a wildcard selector to target this specific widget
        std::string wrapped_css = "* { " + css_style + " }";
        
        gtk_css_provider_load_from_data(inline_provider, wrapped_css.c_str(), -1);
        
        // Use priority_user to override the external stylesheet
        gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(inline_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
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