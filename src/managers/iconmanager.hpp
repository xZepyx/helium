#pragma once
#include <gtk/gtk.h>
#include <string>
#include <vector>
#include "cssmanager.hpp"

class IconManager {
private:
    GtkIconTheme* _icon_theme;
    std::vector<std::string> _added_paths;
    static IconManager* _instance;

    IconManager() {
        _icon_theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
    }

public:
    static IconManager& get_default() {
        if (!_instance) _instance = new IconManager();
        return *_instance;
    }

    void add_icons(const std::string& path) {
        gtk_icon_theme_add_search_path(_icon_theme, path.c_str());
        _added_paths.push_back(path);
    }

    void remove_icons(const std::string& path) {
        auto it = std::find(_added_paths.begin(), _added_paths.end(), path);
        if (it != _added_paths.end()) {
            _added_paths.erase(it);
        }
    }

    std::vector<std::string> get_added_icons() { return _added_paths; }
};

IconManager* IconManager::_instance = nullptr;
