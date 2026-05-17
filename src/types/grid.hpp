#pragma once

#include <gtk/gtk.h>

#include <string>
#include <vector>

#include "widget.hpp"

struct GridProperties {
    bool homogeneous = false;
    int column_spacing = 0;
    int row_spacing = 0;
    std::string halign = "fill";
    std::string valign = "fill";
    bool hexpand = false;
    bool vexpand = false;
};

class Grid : public Widget {
public:
    Grid(const GridProperties& props = {})
        : Widget(gtk_grid_new())
    {
        gtk_grid_set_column_homogeneous(GTK_GRID(native), props.homogeneous);
        gtk_grid_set_column_spacing(GTK_GRID(native), props.column_spacing);
        gtk_grid_set_row_spacing(GTK_GRID(native), props.row_spacing);

        set_align(props.halign, true);
        set_align(props.valign, false);

        gtk_widget_set_hexpand(native, props.hexpand);
        gtk_widget_set_vexpand(native, props.vexpand);
    }

    void attach(Widget* child, int column, int row, int width = 1, int height = 1) {
        gtk_grid_attach(GTK_GRID(native), child->get_native(), column, row, width, height);
    }

    void remove(Widget* child) {
        gtk_grid_remove(GTK_GRID(native), child->get_native());
    }

    void set_column_homogeneous(bool v) {
        gtk_grid_set_column_homogeneous(GTK_GRID(native), v);
    }

    void set_row_homogeneous(bool v) {
        gtk_grid_set_row_homogeneous(GTK_GRID(native), v);
    }

    void set_column_spacing(int s) {
        gtk_grid_set_column_spacing(GTK_GRID(native), s);
    }

    void set_row_spacing(int s) {
        gtk_grid_set_row_spacing(GTK_GRID(native), s);
    }

    void insert_column(int position) {
        gtk_grid_insert_column(GTK_GRID(native), position);
    }

    void insert_row(int position) {
        gtk_grid_insert_row(GTK_GRID(native), position);
    }

    void remove_column(int position) {
        gtk_grid_remove_column(GTK_GRID(native), position);
    }

    void remove_row(int position) {
        gtk_grid_remove_row(GTK_GRID(native), position);
    }

private:
    void set_align(const std::string& align, bool horizontal) {
        GtkAlign g_align = GTK_ALIGN_FILL;
        if (align == "start") g_align = GTK_ALIGN_START;
        else if (align == "center") g_align = GTK_ALIGN_CENTER;
        else if (align == "end") g_align = GTK_ALIGN_END;
        if (horizontal) gtk_widget_set_halign(native, g_align);
        else gtk_widget_set_valign(native, g_align);
    }
};
