#pragma once

#include <gtk/gtk.h>

#include <string>

#include "widget.hpp"

class Corner : public Widget {
private:
    int _size = 20;
    int _radius = 10;
    std::string _corner = "top-left";
    std::string _color = "black";

    static void draw_cb(GtkDrawingArea* area, cairo_t* cr, int w, int h, gpointer data) {
        auto self = static_cast<Corner*>(data);
        if (!self) return;

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
        cairo_paint(cr);

        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        {
            GdkRGBA rgba;
            if (gdk_rgba_parse(&rgba, self->_color.c_str())) {
                cairo_set_source_rgba(cr, rgba.red, rgba.green, rgba.blue, rgba.alpha);
            } else {
                cairo_set_source_rgb(cr, 0, 0, 0);
            }
        }

        double s = self->_size;
        double r = self->_radius;

        cairo_new_path(cr);

        if (self->_corner == "top-left") {
            cairo_arc(cr, r, r, r, G_PI, 3 * G_PI / 2);
            cairo_line_to(cr, 0, s);
            cairo_line_to(cr, s, s);
            cairo_line_to(cr, s, 0);
        } else if (self->_corner == "top-right") {
            cairo_arc(cr, s - r, r, r, 3 * G_PI / 2, 2 * G_PI);
            cairo_line_to(cr, s, s);
            cairo_line_to(cr, 0, s);
            cairo_line_to(cr, 0, 0);
        } else if (self->_corner == "bottom-left") {
            cairo_arc(cr, r, s - r, r, G_PI / 2, G_PI);
            cairo_line_to(cr, 0, 0);
            cairo_line_to(cr, s, 0);
            cairo_line_to(cr, s, s);
        } else if (self->_corner == "bottom-right") {
            cairo_arc(cr, s - r, s - r, r, 0, G_PI / 2);
            cairo_line_to(cr, s, 0);
            cairo_line_to(cr, 0, 0);
            cairo_line_to(cr, 0, s);
        }

        cairo_close_path(cr);
        cairo_fill(cr);
    }

    void redraw() {
        gtk_widget_queue_draw(native);
    }

public:
    Corner(int size = 20, int radius = 10, const std::string& corner = "top-left", const std::string& color = "black")
        : Widget(gtk_drawing_area_new()),
          _size(size), _radius(radius), _corner(corner), _color(color)
    {
        gtk_widget_set_size_request(native, _size, _size);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(native), draw_cb, this, nullptr);
    }

    void set_size(int s) { _size = s; gtk_widget_set_size_request(native, _size, _size); redraw(); }
    int get_size() { return _size; }

    void set_radius(int r) { _radius = r; redraw(); }
    int get_radius() { return _radius; }

    void set_corner(const std::string& c) { _corner = c; redraw(); }
    std::string get_corner() { return _corner; }

    void set_color(const std::string& c) { _color = c; redraw(); }
    std::string get_color() { return _color; }
};
