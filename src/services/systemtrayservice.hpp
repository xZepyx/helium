#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <glib.h>

struct TrayItem {
    std::string id;
    std::string title;
    std::string category;
    std::string status;
    uint32_t window_id = 0;
    bool attention = false;
};

class SystemTrayService {
private:
    static SystemTrayService* _instance;

    std::vector<TrayItem> _items;
    bool _available = false;
    std::string _dbus_address;

    std::map<std::string, std::vector<std::function<void()>>> _signals;
    guint _poll_id = 0;

    SystemTrayService() {}

public:
    static SystemTrayService& get_default() {
        if (!_instance) _instance = new SystemTrayService();
        return *_instance;
    }

    void connect_signal(const std::string& signal, std::function<void()> callback) {
        _signals[signal].push_back(callback);
    }

    void emit(const std::string& signal) {
        if (_signals.count(signal)) {
            for (auto& cb : _signals[signal]) {
                if (cb) cb();
            }
        }
    }

    bool is_available() const {
        return _available;
    }

    std::vector<TrayItem> get_items() const {
        return _items;
    }

    void set_dbus_address(const std::string& addr) {
        _dbus_address = addr;
    }

    std::string get_dbus_address() const {
        return _dbus_address;
    }

    void start_polling(uint32_t interval_ms = 5000) {
        if (_poll_id > 0) return;
        _poll_id = g_timeout_add(interval_ms, [](gpointer data) -> gboolean {
            static_cast<SystemTrayService*>(data)->poll_dbus();
            return TRUE;
        }, this);
    }

    void stop_polling() {
        if (_poll_id > 0) {
            g_source_remove(_poll_id);
            _poll_id = 0;
        }
    }

private:
    void poll_dbus() {
        // Placeholder: DBus system tray (SNI) integration requires
        // a DBus library (e.g., dbus-cxx, sdbus-c++, or GDBus).
        // This method will be implemented once DBus integration
        // is added to the project.
        //
        // Future implementation will:
        //  1. Connect to the DBus session bus
        //  2. Register as a StatusNotifierWatcher
        //  3. Listen for StatusNotifierItem registrations
        //  4. Track items and their properties
        //  5. Forward icon/click events to widgets
    }
};

SystemTrayService* SystemTrayService::_instance = nullptr;
