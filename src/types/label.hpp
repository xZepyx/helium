#pragma once

#include <string>

#include "widget.hpp"

class Label : public Widget {
private:
    std::string text;

public:
    Label(const std::string& label)
        : Widget(gtk_label_new(label.c_str())),
          text(label)
    {}

    void set_label(
        const std::string& new_text
    ) {
        text = new_text;

        gtk_label_set_text(
            GTK_LABEL(native),
            text.c_str()
        );
    }

    std::string get_label() {
        return text;
    }
};