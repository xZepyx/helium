#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <string>

#include "widget.hpp"

class SpinButton : public Widget {
private:
    double _min = 0;
    double _max = 100;
    double _step = 1;
    double _value = 0;
    GtkAdjustment* _adjustment;

    std::function<void(double)> _on_change;

    static void change_cb(GtkSpinButton* button, gpointer data) {
        auto self = static_cast<SpinButton*>(data);
        if (!self || !self->_on_change) return;
        self->_value = gtk_spin_button_get_value(button);
        self->_on_change(self->_value);
    }

public:
    SpinButton(double min = 0, double max = 100, double step = 1, double value = 0)
        : Widget(nullptr),
          _min(min), _max(max), _step(step), _value(value)
    {
        _adjustment = gtk_adjustment_new(value, min, max, step, step * 10, 0);
        native = gtk_spin_button_new(_adjustment, step, 0);
        g_object_ref_sink(native);
    }

    void set_min(double min) {
        _min = min;
        gtk_adjustment_set_lower(_adjustment, min);
    }

    double get_min() { return _min; }

    void set_max(double max) {
        _max = max;
        gtk_adjustment_set_upper(_adjustment, max);
    }

    double get_max() { return _max; }

    void set_step(double step) {
        _step = step;
        gtk_adjustment_set_step_increment(_adjustment, step);
    }

    double get_step() { return _step; }

    void set_value(double value) {
        _value = value;
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(native), value);
    }

    double get_value() { return _value; }

    int get_value_as_int() {
        return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(native));
    }

    void set_digits(int digits) {
        gtk_spin_button_set_digits(GTK_SPIN_BUTTON(native), digits);
    }

    void set_wrap(bool wrap) {
        gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(native), wrap);
    }

    void on_change(std::function<void(double)> callback) {
        _on_change = callback;
        g_signal_connect(native, "value-changed", G_CALLBACK(change_cb), this);
    }
};
