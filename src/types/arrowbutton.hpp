#pragma once

#include <string>

#include "button.hpp"
#include "arrow.hpp"

class ArrowButton : public Button {
private:
    Arrow* _arrow;

public:
    ArrowButton(Arrow* arrow, const std::string& label = "")
        : Button({.label = label}), _arrow(arrow)
    {
        if (!label.empty()) {
            auto box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
            auto lbl = gtk_label_new(label.c_str());
            gtk_box_append(GTK_BOX(box), arrow->get_native());
            gtk_box_append(GTK_BOX(box), lbl);
            gtk_button_set_child(GTK_BUTTON(native), box);
        } else {
            gtk_button_set_child(GTK_BUTTON(native), arrow->get_native());
        }
    }

    Arrow* get_arrow() { return _arrow; }

    void toggle() { _arrow->toggle(); }
};
