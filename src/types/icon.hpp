#pragma once

#include <gtk/gtk.h>

#include <string>

#include "widget.hpp"

class Icon : public Widget {
public:
    Icon(int pixel_size = -1)
        : Widget(gtk_image_new())
    {
        if (pixel_size > 0) {
            gtk_image_set_pixel_size(GTK_IMAGE(native), pixel_size);
        }
    }

    void set_icon_name(const std::string& name) {
        gtk_image_set_from_icon_name(GTK_IMAGE(native), name.c_str());
    }

    std::string get_icon_name() {
        return gtk_image_get_icon_name(GTK_IMAGE(native));
    }

    void set_from_file(const std::string& path) {
        gtk_image_set_from_file(GTK_IMAGE(native), path.c_str());
    }

    void set_pixel_size(int size) {
        gtk_image_set_pixel_size(GTK_IMAGE(native), size);
    }

    int get_pixel_size() {
        return gtk_image_get_pixel_size(GTK_IMAGE(native));
    }

    static Icon* from_icon_name(const std::string& name, int size = -1) {
        auto icon = new Icon(size);
        icon->set_icon_name(name);
        return icon;
    }

    static Icon* from_file(const std::string& path, int size = -1) {
        auto icon = new Icon(size);
        icon->set_from_file(path);
        return icon;
    }
};
