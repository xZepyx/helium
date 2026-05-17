#pragma once

#include <gtk/gtk.h>

#include "widget.hpp"

class Revealer : public Widget {
public:
    Revealer()
        : Widget(gtk_revealer_new())
    {
        gtk_revealer_set_transition_type(GTK_REVEALER(native), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
        gtk_revealer_set_transition_duration(GTK_REVEALER(native), 200);
    }

    void set_child(Widget* child) override {
        gtk_revealer_set_child(GTK_REVEALER(native), child->get_native());
    }

    void set_reveal_child(bool reveal) {
        gtk_revealer_set_reveal_child(GTK_REVEALER(native), reveal);
    }

    bool get_reveal_child() {
        return gtk_revealer_get_reveal_child(GTK_REVEALER(native));
    }

    void set_transition_duration(guint duration) {
        gtk_revealer_set_transition_duration(GTK_REVEALER(native), duration);
    }

    guint get_transition_duration() {
        return gtk_revealer_get_transition_duration(GTK_REVEALER(native));
    }

    void set_transition_type(const std::string& type) {
        GtkRevealerTransitionType t = GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN;
        if (type == "none") t = GTK_REVEALER_TRANSITION_TYPE_NONE;
        else if (type == "crossfade") t = GTK_REVEALER_TRANSITION_TYPE_CROSSFADE;
        else if (type == "slide_right") t = GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT;
        else if (type == "slide_left") t = GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT;
        else if (type == "slide_up") t = GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP;
        else if (type == "slide_down") t = GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN;
        gtk_revealer_set_transition_type(GTK_REVEALER(native), t);
    }

    std::string get_transition_type() {
        auto t = gtk_revealer_get_transition_type(GTK_REVEALER(native));
        switch (t) {
            case GTK_REVEALER_TRANSITION_TYPE_NONE: return "none";
            case GTK_REVEALER_TRANSITION_TYPE_CROSSFADE: return "crossfade";
            case GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT: return "slide_right";
            case GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT: return "slide_left";
            case GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP: return "slide_up";
            case GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN: return "slide_down";
            default: return "slide_down";
        }
    }

    void toggle() {
        gtk_revealer_set_reveal_child(GTK_REVEALER(native),
            !gtk_revealer_get_reveal_child(GTK_REVEALER(native)));
    }
};
