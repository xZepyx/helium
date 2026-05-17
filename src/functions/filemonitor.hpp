#pragma once
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <string>
#include <functional>
#include <map>

struct FileMonitorEntry {
    GFileMonitor* monitor = nullptr;
};

class FileMonitor {
private:
    static std::map<std::string, FileMonitorEntry>& get_entries() {
        static std::map<std::string, FileMonitorEntry> entries;
        return entries;
    }

    static void on_changed(GFileMonitor* monitor, GFile* file, GFile* other,
                           GFileMonitorEvent event_type, gpointer user_data) {
        auto data = static_cast<std::pair<std::string, std::function<void(std::string, std::string)>>*>(user_data);
        std::string event_str;
        switch (event_type) {
            case G_FILE_MONITOR_EVENT_CHANGED: event_str = "changed"; break;
            case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT: event_str = "changes_done_hint"; break;
            case G_FILE_MONITOR_EVENT_DELETED: event_str = "deleted"; break;
            case G_FILE_MONITOR_EVENT_CREATED: event_str = "created"; break;
            default: event_str = "other"; break;
        }
        if (data->second) data->second(g_file_peek_path(file) ? g_file_peek_path(file) : "", event_str);
    }

public:
    static void watch(const std::string& path, std::function<void(std::string, std::string)> callback, bool recursive = false) {
        auto& entries = get_entries();
        if (entries.find(path) != entries.end()) return;

        GFile* gfile = g_file_new_for_path(path.c_str());
        GFileMonitor* monitor = g_file_monitor_file(gfile, G_FILE_MONITOR_NONE, nullptr, nullptr);

        auto* data = new std::pair<std::string, std::function<void(std::string, std::string)>>(path, callback);
        g_signal_connect(monitor, "changed", G_CALLBACK(on_changed), data);

        entries[path] = {monitor};
        g_object_unref(gfile);
    }

    static void unwatch(const std::string& path) {
        auto& entries = get_entries();
        auto it = entries.find(path);
        if (it != entries.end()) {
            g_object_unref(it->second.monitor);
            entries.erase(it);
        }
    }
};
