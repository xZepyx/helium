#pragma once

#include <gtk/gtk.h>

#include <string>

#include "widget.hpp"

class Picture : public Widget {
private:
    std::string _filename;

public:
    Picture()
        : Widget(gtk_picture_new())
    {}

    explicit Picture(const std::string& filename)
        : Widget(gtk_picture_new_for_filename(filename.c_str())), _filename(filename)
    {}

    void set_filename(const std::string& filename) {
        _filename = filename;
        gtk_picture_set_filename(GTK_PICTURE(native), filename.c_str());
    }

    std::string get_filename() {
        return _filename;
    }

    void set_paintable(GdkPaintable* paintable) {
        gtk_picture_set_paintable(GTK_PICTURE(native), paintable);
    }

    void set_keep_aspect_ratio(bool keep) {
        gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(native), keep);
    }

    bool get_keep_aspect_ratio() {
        return gtk_picture_get_keep_aspect_ratio(GTK_PICTURE(native));
    }

    void set_can_shrink(bool shrink) {
        gtk_picture_set_can_shrink(GTK_PICTURE(native), shrink);
    }

    bool get_can_shrink() {
        return gtk_picture_get_can_shrink(GTK_PICTURE(native));
    }
};
