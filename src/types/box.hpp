#pragma once

#include <gtk/gtk.h>

#include <vector>
#include <string>

#include "widget.hpp"

struct BoxProperties {
    std::string orientation =
        "horizontal";

    int spacing = 0;

    std::vector<Widget*> children;
};

class Box : public Widget {
public:
    Box(const BoxProperties& props)
        : Widget(nullptr)
    {
        GtkOrientation orientation =
            GTK_ORIENTATION_HORIZONTAL;

        if (
            props.orientation ==
            "vertical"
        ) {
            orientation =
                GTK_ORIENTATION_VERTICAL;
        }

        native =
            gtk_box_new(
                orientation,
                props.spacing
            );

        for (
            auto child :
            props.children
        ) {
            add(child);
        }
    }

    void add(Widget* child) {
        gtk_box_append(
            GTK_BOX(native),
            child->get_native()
        );
    }
};