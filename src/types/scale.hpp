#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <string>

#include "widget.hpp"

class Scale : public Widget {
private:
    double _min = 0;
    double _max = 100;
    double _step = 1;
    double _value = 0;

    gulong _change_handler = 0;
    std::function<void(double)> _on_change;

    static void change_cb(GtkRange* range, gpointer data) {
        auto self = static_cast<Scale*>(data);
        if (!self || !self->_on_change) return;
        self->_value = gtk_range_get_value(range);
        self->_on_change(self->_value);
    }

public:
    Scale(double min = 0, double max = 100, double step = 1, double value = 0,
          const std::string& orientation = "horizontal")
        : Widget(gtk_scale_new_with_range(
            orientation == "vertical" ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL,
            min, max, step)),
          _min(min), _max(max), _step(step), _value(value)
    {
        gtk_range_set_value(GTK_RANGE(native), value);
        gtk_scale_set_digits(GTK_SCALE(native), 0);
        gtk_scale_set_has_origin(GTK_SCALE(native), TRUE);
    }

    void set_min(double min) { _min = min; gtk_range_set_range(GTK_RANGE(native), _min, _max); }
    double get_min() { return _min; }

    void set_max(double max) { _max = max; gtk_range_set_range(GTK_RANGE(native), _min, _max); }
    double get_max() { return _max; }

    void set_step(double step) { _step = step; gtk_range_set_increments(GTK_RANGE(native), _step, _step * 10); }
    double get_step() { return _step; }

    void set_value(double value) {
        _value = value;
        gtk_range_set_value(GTK_RANGE(native), value);
    }

    double get_value() { return _value; }

    void set_digits(int digits) {
        gtk_scale_set_digits(GTK_SCALE(native), digits);
    }

    void on_change(std::function<void(double)> callback) {
        _on_change = callback;
        if (_change_handler) g_signal_handler_disconnect(native, _change_handler);
        _change_handler = g_signal_connect(native, "value-changed", G_CALLBACK(change_cb), this);
    }
};
