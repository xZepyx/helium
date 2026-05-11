#pragma once
#include <gtk/gtk.h>
#include <map>
#include <string>
#include <iostream>

class CSS_MANAGER {
public:
    struct StyleEntry {
        GtkCssProvider* provider = nullptr;
        GFileMonitor* monitor = nullptr;
    };

    // Singleton map to prevent the "Static Initialization Order Fiasco"
    static std::map<std::string, StyleEntry>& get_entries() {
        static std::map<std::string, StyleEntry> instance;
        return instance;
    }

    static void load(const char* path) {
        std::string p = path;
        auto& entries = get_entries();

        // Avoid double-loading the same file
        if (entries.find(p) != entries.end()) return;

        // 1. Safety Check: Is GTK actually initialized?
        GdkDisplay* display = gdk_display_get_default();
        if (!display) {
            g_warning("CSS_MANAGER: Cannot load CSS. GDK Display is null. Did you call helium.init()?");
            return;
        }

        StyleEntry entry;
        entry.provider = gtk_css_provider_new();

        // 2. Connect provider to the display
        gtk_style_context_add_provider_for_display(
            display,
            GTK_STYLE_PROVIDER(entry.provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );

        // 3. Initial load
        gtk_css_provider_load_from_path(entry.provider, path);

        // 4. Set up File Monitoring for Hot-Reload
        GFile* file = g_file_new_for_path(path);
        entry.monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, nullptr, nullptr);
        
        // Pass the path string as user_data so the callback knows which provider to update
        // Note: In a production app, you'd want to manage this memory, 
        // but for a shell manager, this persists for the app lifetime.
        g_signal_connect(entry.monitor, "changed", G_CALLBACK(on_file_changed), g_strdup(path));
        
        entries[p] = entry;
        g_object_unref(file);
        
        std::cout << "[Helium] CSS Loaded: " << path << std::endl;
    }

private:
    static void on_file_changed(GFileMonitor* monitor, GFile* file, GFile* other, 
                               GFileMonitorEvent event, gpointer user_data) {
        // Only reload on specific events to avoid "double-reload" flickering
        if (event == G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT || event == G_FILE_MONITOR_EVENT_CREATED) {
            const char* path = static_cast<const char*>(user_data);
            auto& entries = get_entries();
            
            if (entries.count(path)) {
                std::cout << "[Helium] CSS Hot-Reloading: " << path << std::endl;
                gtk_css_provider_load_from_path(entries[path].provider, path);
            }
        }
    }
};