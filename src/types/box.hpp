#pragma once

#include <gtk/gtk.h>
#include <vector>
#include <string>
#include "widget.hpp"

struct BoxProperties {
    std::string orientation = "horizontal";
    int spacing = 0;
    std::vector<Widget*> children;
    std::string halign = "fill"; 
    std::string valign = "fill";
    bool hexpand = false;
    bool vexpand = false;
};

class Box : public Widget {
public:
    Box(const BoxProperties& props) : Widget(nullptr) {
        GtkOrientation orientation = (props.orientation == "vertical") ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
        native = gtk_box_new(orientation, props.spacing);

        set_align(props.halign, true);
        set_align(props.valign, false);

        gtk_widget_set_hexpand(native, props.hexpand);
        gtk_widget_set_vexpand(native, props.vexpand);

        for (auto child : props.children) add(child);
    }

    void add(Widget* child) { gtk_box_append(GTK_BOX(native), child->get_native()); }

private:
    void set_align(const std::string& align, bool horizontal) {
        GtkAlign g_align = GTK_ALIGN_FILL;
        if (align == "start") g_align = GTK_ALIGN_START;
        else if (align == "center") g_align = GTK_ALIGN_CENTER;
        else if (align == "end") g_align = GTK_ALIGN_END;
        if (horizontal) gtk_widget_set_halign(native, g_align); else gtk_widget_set_valign(native, g_align);
    }
};