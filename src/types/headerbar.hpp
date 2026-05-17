#pragma once

#include <gtk/gtk.h>

#include <string>

#include "widget.hpp"

class HeaderBar : public Widget {
private:
    std::string _title;
    std::string _subtitle;

public:
    HeaderBar()
        : Widget(gtk_header_bar_new())
    {
        gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(native), TRUE);
    }

    void set_title(const std::string& title) {
        _title = title;
        auto* label = gtk_label_new(title.c_str());
        gtk_header_bar_set_title_widget(GTK_HEADER_BAR(native), label);
    }

    std::string get_title() {
        return _title;
    }

    void set_subtitle(const std::string& subtitle) {
        _subtitle = subtitle;
    }

    std::string get_subtitle() {
        return _subtitle;
    }

    void set_show_title_buttons(bool show) {
        gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(native), show);
    }

    bool get_show_title_buttons() {
        return gtk_header_bar_get_show_title_buttons(GTK_HEADER_BAR(native));
    }

    void pack_start(Widget* child) {
        gtk_header_bar_pack_start(GTK_HEADER_BAR(native), child->get_native());
    }

    void pack_end(Widget* child) {
        gtk_header_bar_pack_end(GTK_HEADER_BAR(native), child->get_native());
    }

    void remove(Widget* child) {
        gtk_header_bar_remove(GTK_HEADER_BAR(native), child->get_native());
    }
};
