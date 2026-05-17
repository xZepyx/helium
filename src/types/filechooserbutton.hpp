#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <string>
#include <vector>

#include "widget.hpp"
#include "filefilter.hpp"

class FileChooserButton : public Widget {
private:
    std::string _title;
    std::string _action = "open";
    std::vector<FileFilter> _filters;
    std::function<void(std::string)> _on_file_set;

    static void file_set_cb(GtkFileChooser* chooser, gpointer data) {
        auto self = static_cast<FileChooserButton*>(data);
        if (!self || !self->_on_file_set) return;
        GFile* file = gtk_file_chooser_get_file(chooser);
        if (file) {
            char* path = g_file_get_path(file);
            if (path) {
                self->_on_file_set(std::string(path));
                g_free(path);
            }
            g_object_unref(file);
        }
    }

public:
    FileChooserButton(const std::string& title = "Select File",
                      const std::string& action = "open")
        : Widget(nullptr), _title(title), _action(action)
    {
        GtkFileChooserAction g_action = GTK_FILE_CHOOSER_ACTION_OPEN;
        if (action == "save") g_action = GTK_FILE_CHOOSER_ACTION_SAVE;
        else if (action == "select-folder") g_action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
        else if (action == "create-folder") g_action = GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER;

        auto dialog = gtk_file_chooser_native_new(title.c_str(), nullptr, g_action, nullptr, nullptr);
        native = gtk_file_chooser_widget_new(g_action);
        g_object_ref_sink(native);
        g_object_unref(dialog);

        g_signal_connect(native, "file-set", G_CALLBACK(file_set_cb), this);
    }

    void set_title(const std::string& title) {
        _title = title;
    }

    std::string get_title() { return _title; }

    void set_action(const std::string& action) {
        _action = action;
    }

    std::string get_action() { return _action; }

    void add_filter(const FileFilter& filter) {
        _filters.push_back(filter);
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter.get_native());
    }

    void set_current_name(const std::string& name) {
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(native), name.c_str());
    }

    void set_current_folder(const std::string& path) {
        GFile* file = g_file_new_for_path(path.c_str());
        gtk_file_chooser_set_file(GTK_FILE_CHOOSER(native), file, nullptr);
        g_object_unref(file);
    }

    std::string get_filename() {
        GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(native));
        if (!file) return "";
        char* path = g_file_get_path(file);
        std::string result = path ? std::string(path) : "";
        g_free(path);
        g_object_unref(file);
        return result;
    }

    void on_file_set(std::function<void(std::string)> callback) {
        _on_file_set = callback;
    }

    void set_modal(bool modal) {
        gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(native), modal);
    }
};
