#pragma once
#include "widget.hpp"

class CenterBox : public Widget {
public:
    CenterBox() : Widget(gtk_center_box_new()) {}

    void set_start(Widget* child) { gtk_center_box_set_start_widget(GTK_CENTER_BOX(native), child->get_native()); }
    void set_center(Widget* child) { gtk_center_box_set_center_widget(GTK_CENTER_BOX(native), child->get_native()); }
    void set_end(Widget* child) { gtk_center_box_set_end_widget(GTK_CENTER_BOX(native), child->get_native()); }
};