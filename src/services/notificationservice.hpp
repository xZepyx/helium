#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <algorithm>
#include <glib.h>

struct Notification {
    uint32_t id = 0;
    std::string app_name;
    std::string summary;
    std::string body;
    std::string icon;
    int32_t urgency = 0;
    uint32_t timestamp = 0;
    std::string category;
    bool popup = true;
};

class NotificationService {
private:
    static NotificationService* _instance;

    std::vector<Notification> _notifications;
    std::vector<Notification> _popups;
    std::vector<uint32_t> _dismissed_ids;
    bool _dnd = false;
    uint32_t _next_id = 1;

    std::map<std::string, std::vector<std::function<void()>>> _signals;

    NotificationService() {}

public:
    static NotificationService& get_default() {
        if (!_instance) _instance = new NotificationService();
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

    uint32_t add_notification(const std::string& app_name, const std::string& summary,
                               const std::string& body, const std::string& icon,
                               int32_t urgency = 0, const std::string& category = "") {
        if (_dnd) return 0;

        Notification notif;
        notif.id = _next_id++;
        notif.app_name = app_name;
        notif.summary = summary;
        notif.body = body;
        notif.icon = icon;
        notif.urgency = urgency;
        notif.timestamp = static_cast<uint32_t>(time(nullptr));
        notif.category = category;
        notif.popup = true;

        _notifications.push_back(notif);
        _popups.push_back(notif);

        emit("new-notification");

        return notif.id;
    }

    void dismiss(uint32_t id) {
        _dismissed_ids.push_back(id);
        _popups.erase(
            std::remove_if(_popups.begin(), _popups.end(),
                [id](const Notification& n) { return n.id == id; }),
            _popups.end()
        );
        emit("dismissed");
    }

    void dismiss_all() {
        for (auto& n : _popups) {
            _dismissed_ids.push_back(n.id);
        }
        _popups.clear();
        emit("dismissed");
    }

    std::vector<Notification> get_notifications() const {
        std::vector<Notification> result;
        for (auto& n : _notifications) {
            if (std::find(_dismissed_ids.begin(), _dismissed_ids.end(), n.id) == _dismissed_ids.end()) {
                result.push_back(n);
            }
        }
        return result;
    }

    std::vector<Notification> get_popups() const {
        return _popups;
    }

    bool get_dnd() const { return _dnd; }

    void set_dnd(bool enabled) {
        _dnd = enabled;
        if (enabled) {
            _popups.clear();
        }
        emit("dnd-changed");
    }

    void toggle_dnd() {
        set_dnd(!_dnd);
    }

    void clear_all() {
        _notifications.clear();
        _popups.clear();
        _dismissed_ids.clear();
        emit("cleared");
    }

    size_t get_count() const {
        return _notifications.size() - _dismissed_ids.size();
    }

    size_t get_popup_count() const {
        return _popups.size();
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }
};

NotificationService* NotificationService::_instance = nullptr;
