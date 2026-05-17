#pragma once
#include <gtk/gtk.h>
#include <map>
#include <string>
#include <vector>
#include <iostream>

enum class StylePriority {
    Fallback = GTK_STYLE_PROVIDER_PRIORITY_FALLBACK,
    Theme = GTK_STYLE_PROVIDER_PRIORITY_THEME,
    User = GTK_STYLE_PROVIDER_PRIORITY_USER,
    Application = GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
};

struct CssInfoString {
    std::string css;
    StylePriority priority = StylePriority::Application;
};

struct CssInfoPath {
    std::string path;
    StylePriority priority = StylePriority::Application;
};

class CSS_MANAGER {
public:
    struct StyleEntry {
        GtkCssProvider* provider = nullptr;
        GFileMonitor* monitor = nullptr;
        StylePriority priority = StylePriority::Application;
    };

    static std::map<std::string, StyleEntry>& get_entries() {
        static std::map<std::string, StyleEntry> instance;
        return instance;
    }

    static void load(const char* path) {
        apply(CssInfoPath{path, StylePriority::Application});
    }

    static void load(const char* path, StylePriority priority) {
        apply(CssInfoPath{path, priority});
    }

    static void load_string(const std::string& css, StylePriority priority = StylePriority::Application) {
        apply(CssInfoString{css, priority});
    }

    static void apply(const CssInfoString& info) {
        GdkDisplay* display = gdk_display_get_default();
        if (!display) {
            g_warning("CSS_MANAGER: Cannot apply CSS. GDK Display is null.");
            return;
        }

        GtkCssProvider* provider = gtk_css_provider_new();
        gtk_css_provider_load_from_string(provider, info.css.c_str());
        gtk_style_context_add_provider_for_display(
            display,
            GTK_STYLE_PROVIDER(provider),
            static_cast<guint>(info.priority)
        );

        std::string id = "_string_" + std::to_string(reinterpret_cast<uintptr_t>(provider));
        get_entries()[id] = {provider, nullptr, info.priority};
    }

    static void apply(const CssInfoPath& info) {
        std::string p = info.path;
        auto& entries = get_entries();

        if (entries.find(p) != entries.end()) return;

        GdkDisplay* display = gdk_display_get_default();
        if (!display) {
            g_warning("CSS_MANAGER: Cannot load CSS. GDK Display is null. Did you call helium.init()?");
            return;
        }

        StyleEntry entry;
        entry.provider = gtk_css_provider_new();
        entry.priority = info.priority;

        gtk_style_context_add_provider_for_display(
            display,
            GTK_STYLE_PROVIDER(entry.provider),
            static_cast<guint>(info.priority)
        );

        gtk_css_provider_load_from_path(entry.provider, info.path.c_str());

        GFile* file = g_file_new_for_path(info.path.c_str());
        entry.monitor = g_file_monitor_file(file, G_FILE_MONITOR_NONE, nullptr, nullptr);

        g_signal_connect(entry.monitor, "changed", G_CALLBACK(on_file_changed), g_strdup(info.path.c_str()));

        entries[p] = entry;
        g_object_unref(file);

        std::cout << "[Helium] CSS Loaded: " << info.path << std::endl;
    }

    static void remove(const std::string& identifier) {
        auto& entries = get_entries();
        auto it = entries.find(identifier);
        if (it != entries.end()) {
            GdkDisplay* display = gdk_display_get_default();
            if (display && it->second.provider) {
                gtk_style_context_remove_provider_for_display(
                    display,
                    GTK_STYLE_PROVIDER(it->second.provider)
                );
            }
            if (it->second.monitor) {
                g_object_unref(it->second.monitor);
            }
            g_object_unref(it->second.provider);
            entries.erase(it);
            std::cout << "[Helium] CSS Removed: " << identifier << std::endl;
        }
    }

    static void reload() {
        auto& entries = get_entries();
        for (auto& pair : entries) {
            if (!pair.second.monitor) continue;
            gtk_css_provider_load_from_path(pair.second.provider, pair.first.c_str());
        }
        std::cout << "[Helium] CSS Reloaded all files" << std::endl;
    }

    static void reset() {
        auto& entries = get_entries();
        GdkDisplay* display = gdk_display_get_default();
        for (auto& pair : entries) {
            if (display && pair.second.provider) {
                gtk_style_context_remove_provider_for_display(
                    display,
                    GTK_STYLE_PROVIDER(pair.second.provider)
                );
            }
            if (pair.second.monitor) {
                g_object_unref(pair.second.monitor);
            }
            g_object_unref(pair.second.provider);
        }
        entries.clear();
        std::cout << "[Helium] CSS Reset all styles" << std::endl;
    }

private:
    static void on_file_changed(GFileMonitor* monitor, GFile* file, GFile* other,
                               GFileMonitorEvent event, gpointer user_data) {
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
