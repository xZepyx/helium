#pragma once
#include "widget.hpp"

class Image : public Widget {
public:
    // Load from icon theme (e.g., "folder-music")
    static Image* from_icon(const std::string& name) {
        return new Image(gtk_image_new_from_icon_name(name.c_str()));
    }

    // Load from file path
    static Image* from_file(const std::string& path) {
        return new Image(gtk_image_new_from_file(path.c_str()));
    }

private:
    Image(GtkWidget* w) : Widget(w) {}
};