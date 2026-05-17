#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <string>
#include <vector>

#include "widget.hpp"
#include "filefilter.hpp"

class FileDialog {
private:
    GtkFileDialog* _dialog;
    std::function<void(std::string)> _on_open;
    std::function<void(std::string)> _on_save;
    std::vector<FileFilter> _filters;

    static void open_cb(GObject* source, GAsyncResult* res, gpointer data) {
        auto self = static_cast<FileDialog*>(data);
        if (!self || !self->_on_open) return;

        GFile* file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source), res, nullptr);
        if (file) {
            char* path = g_file_get_path(file);
            if (path) {
                self->_on_open(std::string(path));
                g_free(path);
            }
            g_object_unref(file);
        }
    }

    static void save_cb(GObject* source, GAsyncResult* res, gpointer data) {
        auto self = static_cast<FileDialog*>(data);
        if (!self || !self->_on_save) return;

        GFile* file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(source), res, nullptr);
        if (file) {
            char* path = g_file_get_path(file);
            if (path) {
                self->_on_save(std::string(path));
                g_free(path);
            }
            g_object_unref(file);
        }
    }

public:
    FileDialog()
        : _dialog(gtk_file_dialog_new())
    {}

    ~FileDialog() {
        g_object_unref(_dialog);
    }

    void open(Widget* parent) {
        if (!parent || !parent->get_native()) return;

        GtkWindow* window = GTK_WINDOW(gtk_widget_get_root(parent->get_native()));
        if (!window) {
            window = GTK_WINDOW(parent->get_native());
        }

        gtk_file_dialog_open(_dialog, window, nullptr, open_cb, this);
    }

    void save(Widget* parent, const std::string& suggested_name = "") {
        if (!suggested_name.empty()) {
            GFile* file = g_file_new_for_path(suggested_name.c_str());
            gtk_file_dialog_set_initial_file(_dialog, file);
            g_object_unref(file);
        }

        GtkWindow* window = GTK_WINDOW(gtk_widget_get_root(parent->get_native()));
        if (!window) {
            window = GTK_WINDOW(parent->get_native());
        }

        gtk_file_dialog_save(_dialog, window, nullptr, save_cb, this);
    }

    void set_title(const std::string& title) {
        gtk_file_dialog_set_title(_dialog, title.c_str());
    }

    std::string get_title() {
        return gtk_file_dialog_get_title(_dialog);
    }

    void set_initial_name(const std::string& name) {
        gtk_file_dialog_set_initial_name(_dialog, name.c_str());
    }

    void set_initial_folder(const std::string& path) {
        GFile* file = g_file_new_for_path(path.c_str());
        gtk_file_dialog_set_initial_folder(_dialog, file);
        g_object_unref(file);
    }

    void add_filter(const FileFilter& filter) {
        _filters.push_back(filter);
        auto list_store = gtk_file_dialog_get_filters(_dialog);
        if (!list_store) {
            list_store = g_list_store_new(G_TYPE_OBJECT);
            gtk_file_dialog_set_filters(_dialog, G_LIST_MODEL(list_store));
            g_object_unref(list_store);
        }
        g_list_store_append(G_LIST_STORE(list_store), filter.get_native());
    }

    void on_open(std::function<void(std::string)> callback) {
        _on_open = callback;
    }

    void on_save(std::function<void(std::string)> callback) {
        _on_save = callback;
    }
};
