#pragma once

#include <map>
#include <string>

class Widget;

class WindowManager {
private:
    std::map<std::string, Widget*> _windows;

    WindowManager() = default;

public:
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;

    static WindowManager& get_default() {
        static WindowManager instance;
        return instance;
    }

    void add_window(const std::string& namespace_, Widget* window) {
        _windows[namespace_] = window;
    }

    void remove_window(const std::string& namespace_) {
        _windows.erase(namespace_);
    }

    Widget* get_window(const std::string& namespace_) {
        auto it = _windows.find(namespace_);
        if (it != _windows.end()) return it->second;
        return nullptr;
    }

    bool has_window(const std::string& namespace_) {
        return _windows.find(namespace_) != _windows.end();
    }
};
