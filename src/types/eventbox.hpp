#pragma once
#include <gtk/gtk.h>
#include "widget.hpp"
#include <functional>

class EventBox : public Widget {
private:
    GtkGesture* _click_controller = nullptr;
    GtkGesture* _right_click_controller = nullptr;
    GtkGesture* _middle_click_controller = nullptr;
    GtkEventController* _scroll_controller = nullptr;
    GtkEventController* _motion_controller = nullptr;

    std::function<void()> _on_click;
    std::function<void()> _on_right_click;
    std::function<void()> _on_middle_click;
    std::function<void(bool)> _on_hover;
    std::function<void()> _on_scroll_up;
    std::function<void()> _on_scroll_down;

    static void click_cb(GtkGesture* gesture, int n_press, double x, double y, gpointer data) {
        auto self = static_cast<EventBox*>(data);
        if (self && self->_on_click) {
            gtk_gesture_set_state(gesture, GTK_EVENT_SEQUENCE_CLAIMED);
            self->_on_click();
        }
    }

    static void right_click_cb(GtkGesture* gesture, int n_press, double x, double y, gpointer data) {
        auto self = static_cast<EventBox*>(data);
        if (self && self->_on_right_click) {
            gtk_gesture_set_state(gesture, GTK_EVENT_SEQUENCE_CLAIMED);
            self->_on_right_click();
        }
    }

    static void middle_click_cb(GtkGesture* gesture, int n_press, double x, double y, gpointer data) {
        auto self = static_cast<EventBox*>(data);
        if (self && self->_on_middle_click) {
            gtk_gesture_set_state(gesture, GTK_EVENT_SEQUENCE_CLAIMED);
            self->_on_middle_click();
        }
    }

    static void enter_cb(GtkEventController*, double, double, gpointer data) {
        auto self = static_cast<EventBox*>(data);
        if (self && self->_on_hover) self->_on_hover(true);
    }

    static void leave_cb(GtkEventController*, gpointer data) {
        auto self = static_cast<EventBox*>(data);
        if (self && self->_on_hover) self->_on_hover(false);
    }

    static gboolean scroll_cb(GtkEventController* controller, double dx, double dy, gpointer data) {
        auto self = static_cast<EventBox*>(data);
        if (!self) return FALSE;
        if (dy < 0 && self->_on_scroll_up) self->_on_scroll_up();
        else if (dy > 0 && self->_on_scroll_down) self->_on_scroll_down();
        return TRUE;
    }

public:
    EventBox() : Widget(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0)) {}

    void set_child(Widget* child) override {
        gtk_box_append(GTK_BOX(native), child->get_native());
    }

    void on_hover(std::function<void(bool)> callback) {
        _on_hover = callback;
        if (!_motion_controller) {
            _motion_controller = GTK_EVENT_CONTROLLER(gtk_event_controller_motion_new());
            g_signal_connect(_motion_controller, "enter", G_CALLBACK(enter_cb), this);
            g_signal_connect(_motion_controller, "leave", G_CALLBACK(leave_cb), this);
            gtk_widget_add_controller(native, _motion_controller);
        }
    }

    void on_click(std::function<void()> callback) {
        _on_click = callback;
        if (!_click_controller) {
            _click_controller = GTK_GESTURE(gtk_gesture_click_new());
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(_click_controller), 1);
            g_signal_connect(_click_controller, "pressed", G_CALLBACK(click_cb), this);
            gtk_widget_add_controller(native, GTK_EVENT_CONTROLLER(_click_controller));
        }
    }

    void on_right_click(std::function<void()> callback) {
        _on_right_click = callback;
        if (!_right_click_controller) {
            _right_click_controller = GTK_GESTURE(gtk_gesture_click_new());
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(_right_click_controller), 3);
            g_signal_connect(_right_click_controller, "pressed", G_CALLBACK(right_click_cb), this);
            gtk_widget_add_controller(native, GTK_EVENT_CONTROLLER(_right_click_controller));
        }
    }

    void on_middle_click(std::function<void()> callback) {
        _on_middle_click = callback;
        if (!_middle_click_controller) {
            _middle_click_controller = GTK_GESTURE(gtk_gesture_click_new());
            gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(_middle_click_controller), 2);
            g_signal_connect(_middle_click_controller, "pressed", G_CALLBACK(middle_click_cb), this);
            gtk_widget_add_controller(native, GTK_EVENT_CONTROLLER(_middle_click_controller));
        }
    }

    void on_scroll_up(std::function<void()> callback) {
        _on_scroll_up = callback;
        if (!_scroll_controller) {
            _scroll_controller = GTK_EVENT_CONTROLLER(gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES));
            g_signal_connect(_scroll_controller, "scroll", G_CALLBACK(scroll_cb), this);
            gtk_widget_add_controller(native, _scroll_controller);
        }
    }

    void on_scroll_down(std::function<void()> callback) {
        _on_scroll_down = callback;
        if (!_scroll_controller) {
            _scroll_controller = GTK_EVENT_CONTROLLER(gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES));
            g_signal_connect(_scroll_controller, "scroll", G_CALLBACK(scroll_cb), this);
            gtk_widget_add_controller(native, _scroll_controller);
        }
    }
};
