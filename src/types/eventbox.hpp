#pragma once
#include <gtk/gtk.h>
#include "widget.hpp"
#include <functional>

class EventBox : public Widget {
public:
    EventBox() : Widget(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0)) {}

    void on_hover(std::function<void(bool)> callback) {
        auto controller = gtk_event_controller_motion_new();
        auto cb = new std::function<void(bool)>(callback);

        g_signal_connect_data(controller, "enter", G_CALLBACK(+[](GtkEventControllerMotion*, double, double, gpointer data) {
            (*static_cast<std::function<void(bool)>*>(data))(true);
        }), cb, nullptr, (GConnectFlags)0);

        g_signal_connect_data(controller, "leave", G_CALLBACK(+[](GtkEventControllerMotion*, gpointer data) {
            (*static_cast<std::function<void(bool)>*>(data))(false);
        }), cb, [](gpointer data, GClosure*) {
            delete static_cast<std::function<void(bool)>*>(data);
        }, (GConnectFlags)0);

        gtk_widget_add_controller(native, controller);
    }

    void set_child(Widget* child) override {
        gtk_box_append(GTK_BOX(native), child->get_native());
    }
};