#pragma once

#include <gtk/gtk.h>

#include "stack.hpp"
#include "widget.hpp"

class StackSwitcher : public Widget {
public:
    StackSwitcher()
        : Widget(gtk_stack_switcher_new())
    {}

    void set_stack(Stack* stack) {
        gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(native), GTK_STACK(stack->get_native()));
    }

    Stack* get_stack() {
        auto s = gtk_stack_switcher_get_stack(GTK_STACK_SWITCHER(native));
        if (!s) return nullptr;
        return static_cast<Stack*>(g_object_get_data(G_OBJECT(s), "cpp-widget"));
    }
};
