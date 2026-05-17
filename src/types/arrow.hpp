#pragma once

#include <gtk/gtk.h>

#include <string>

#include "widget.hpp"

enum class ArrowDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

class Arrow : public Widget {
private:
    bool _rotated = false;
    int _degree = 90;
    int _time = 200;
    ArrowDirection _direction = ArrowDirection::UP;
    bool _counterclockwise = false;

    static const char* direction_to_icon_name(ArrowDirection dir) {
        switch (dir) {
            case ArrowDirection::UP:    return "pan-up-symbolic";
            case ArrowDirection::DOWN:  return "pan-down-symbolic";
            case ArrowDirection::LEFT:  return "pan-start-symbolic";
            case ArrowDirection::RIGHT: return "pan-end-symbolic";
        }
        return "pan-up-symbolic";
    }

    void update_rotation() {
        if (_rotated) {
            int base = 0;
            switch (_direction) {
                case ArrowDirection::UP:    base = 0; break;
                case ArrowDirection::RIGHT: base = 90; break;
                case ArrowDirection::DOWN:  base = 180; break;
                case ArrowDirection::LEFT:  base = 270; break;
            }
            int target = _counterclockwise ? (base - _degree) : (base + _degree);
            std::string css = "transform: rotate(" + std::to_string(target) + "deg); "
                              "transition: transform " + std::to_string(_time) + "ms ease;";
            set_style(css);
        } else {
            set_style("transform: rotate(0deg); transition: transform " + std::to_string(_time) + "ms ease;");
        }
    }

public:
    Arrow(ArrowDirection direction = ArrowDirection::UP, int pixel_size = 24)
        : Widget(gtk_image_new_from_icon_name(direction_to_icon_name(direction))),
          _direction(direction)
    {
        gtk_image_set_pixel_size(GTK_IMAGE(native), pixel_size);
        add_css_class("arrow");
    }

    void set_rotated(bool v) { _rotated = v; update_rotation(); }
    bool get_rotated() { return _rotated; }

    void set_degree(int d) { _degree = d; update_rotation(); }
    int get_degree() { return _degree; }

    void set_time(int t) { _time = t; }
    int get_time() { return _time; }

    void set_direction(ArrowDirection d) {
        _direction = d;
        gtk_image_set_from_icon_name(GTK_IMAGE(native), direction_to_icon_name(d));
        update_rotation();
    }

    ArrowDirection get_direction() { return _direction; }

    void set_counterclockwise(bool v) { _counterclockwise = v; update_rotation(); }
    bool get_counterclockwise() { return _counterclockwise; }

    void toggle() {
        _rotated = !_rotated;
        update_rotation();
    }
};
