#pragma once

#include <gtk/gtk.h>

#include <string>

#include "widget.hpp"

class Separator : public Widget {
public:
    Separator(const std::string& orientation = "horizontal")
        : Widget(gtk_separator_new(
            orientation == "vertical" ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL))
    {}
};
