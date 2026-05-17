#pragma once

#include <gtk/gtk.h>

#include <string>
#include <vector>

#include "widget.hpp"
#include "stackpage.hpp"

class Stack : public Widget {
private:
    std::vector<StackPage> _pages;

public:
    Stack()
        : Widget(gtk_stack_new())
    {}

    void add_child(Widget* child, const std::string& title = "") {
        gtk_stack_add_child(GTK_STACK(native), child->get_native());
        _pages.push_back({title, child});

        if (!title.empty()) {
            gtk_widget_set_child_visible(child->get_native(), FALSE);
        }
    }

    void remove(Widget* child) {
        gtk_stack_remove(GTK_STACK(native), child->get_native());
        for (auto it = _pages.begin(); it != _pages.end(); ++it) {
            if (it->child == child) {
                _pages.erase(it);
                break;
            }
        }
    }

    void set_visible_child(Widget* child) {
        gtk_stack_set_visible_child(GTK_STACK(native), child->get_native());
    }

    void set_visible_child_name(const std::string& name) {
        gtk_stack_set_visible_child_name(GTK_STACK(native), name.c_str());
    }

    std::string get_visible_child_name() {
        return gtk_stack_get_visible_child_name(GTK_STACK(native));
    }

    Widget* get_visible_child() {
        auto w = gtk_stack_get_visible_child(GTK_STACK(native));
        if (!w) return nullptr;
        for (auto& p : _pages) {
            if (p.child->get_native() == w) return p.child;
        }
        return nullptr;
    }

    void set_hhomogeneous(bool v) {
        gtk_stack_set_hhomogeneous(GTK_STACK(native), v);
    }

    void set_vhomogeneous(bool v) {
        gtk_stack_set_vhomogeneous(GTK_STACK(native), v);
    }

    void set_transition_type(const std::string& type) {
        GtkStackTransitionType t = GTK_STACK_TRANSITION_TYPE_NONE;
        if (type == "crossfade") t = GTK_STACK_TRANSITION_TYPE_CROSSFADE;
        else if (type == "slide_right") t = GTK_STACK_TRANSITION_TYPE_SLIDE_RIGHT;
        else if (type == "slide_left") t = GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT;
        else if (type == "slide_up") t = GTK_STACK_TRANSITION_TYPE_SLIDE_UP;
        else if (type == "slide_down") t = GTK_STACK_TRANSITION_TYPE_SLIDE_DOWN;
        else if (type == "slide_left_right") t = GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT;
        else if (type == "slide_up_down") t = GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN;
        else if (type == "over_up") t = GTK_STACK_TRANSITION_TYPE_OVER_UP;
        else if (type == "over_down") t = GTK_STACK_TRANSITION_TYPE_OVER_DOWN;
        else if (type == "over_up_down") t = GTK_STACK_TRANSITION_TYPE_OVER_UP_DOWN;
        else if (type == "under_up") t = GTK_STACK_TRANSITION_TYPE_UNDER_UP;
        else if (type == "under_down") t = GTK_STACK_TRANSITION_TYPE_UNDER_DOWN;
        else if (type == "under_left_right") t = GTK_STACK_TRANSITION_TYPE_OVER_LEFT_RIGHT;
        else if (type == "rotate_left") t = GTK_STACK_TRANSITION_TYPE_ROTATE_LEFT;
        else if (type == "rotate_right") t = GTK_STACK_TRANSITION_TYPE_ROTATE_RIGHT;
        gtk_stack_set_transition_type(GTK_STACK(native), t);
    }

    void set_transition_duration(guint ms) {
        gtk_stack_set_transition_duration(GTK_STACK(native), ms);
    }

    const std::vector<StackPage>& get_pages() { return _pages; }
};
