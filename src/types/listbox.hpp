#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <vector>

#include "widget.hpp"
#include "listboxrow.hpp"

class ListBox : public Widget {
private:
    std::vector<ListBoxRow*> _rows;

    static void row_activated_cb(GtkListBox*, GtkListBoxRow* row, gpointer data) {
        auto self = static_cast<ListBox*>(data);
        if (!self) return;
        for (auto r : self->_rows) {
            if (r->get_native() == GTK_WIDGET(row)) {
                r->activate();
                break;
            }
        }
    }

public:
    ListBox()
        : Widget(gtk_list_box_new())
    {
        g_signal_connect(native, "row-activated", G_CALLBACK(row_activated_cb), this);
    }

    void append(ListBoxRow* row) {
        gtk_list_box_append(GTK_LIST_BOX(native), row->get_native());
        _rows.push_back(row);
    }

    void remove(ListBoxRow* row) {
        gtk_list_box_remove(GTK_LIST_BOX(native), row->get_native());
        for (auto it = _rows.begin(); it != _rows.end(); ++it) {
            if (*it == row) {
                _rows.erase(it);
                break;
            }
        }
    }

    void clear() {
        for (auto row : _rows) {
            gtk_list_box_remove(GTK_LIST_BOX(native), row->get_native());
        }
        _rows.clear();
    }

    void select_row(ListBoxRow* row) {
        gtk_list_box_select_row(GTK_LIST_BOX(native), GTK_LIST_BOX_ROW(row->get_native()));
    }

    void unselect_row(ListBoxRow* row) {
        gtk_list_box_unselect_row(GTK_LIST_BOX(native), GTK_LIST_BOX_ROW(row->get_native()));
    }

    ListBoxRow* get_selected_row() {
        auto row = gtk_list_box_get_selected_row(GTK_LIST_BOX(native));
        if (!row) return nullptr;
        for (auto r : _rows) {
            if (r->get_native() == GTK_WIDGET(row)) return r;
        }
        return nullptr;
    }

    void set_selection_mode(const std::string& mode) {
        GtkSelectionMode m = GTK_SELECTION_SINGLE;
        if (mode == "none") m = GTK_SELECTION_NONE;
        else if (mode == "browse") m = GTK_SELECTION_BROWSE;
        else if (mode == "multiple") m = GTK_SELECTION_MULTIPLE;
        gtk_list_box_set_selection_mode(GTK_LIST_BOX(native), m);
    }

    void on_row_activated(std::function<void(ListBoxRow*)> callback) {
        (void)callback;
    }

    const std::vector<ListBoxRow*>& get_rows() { return _rows; }
};
