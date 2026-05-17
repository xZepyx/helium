#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <cstdio>
#include <memory>
#include <array>
#include <sstream>
#include <algorithm>
#include <glib.h>

class PowerProfilesService {
private:
    static PowerProfilesService* _instance;

    std::string _active_profile;
    std::vector<std::string> _profiles;

    std::map<std::string, std::vector<std::function<void()>>> _signals;
    guint _poll_id = 0;

    PowerProfilesService() {}

    static std::string exec(const std::string& cmd) {
        std::array<char, 4096> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) return "";
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

public:
    static PowerProfilesService& get_default() {
        if (!_instance) _instance = new PowerProfilesService();
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

    std::string get_profile() {
        refresh();
        return _active_profile;
    }

    void set_profile(const std::string& profile) {
        exec("powerprofilesctl set " + profile + " 2>/dev/null");
        _active_profile = profile;
        emit("profile-changed");
    }

    std::vector<std::string> get_profiles() {
        refresh();
        return _profiles;
    }

    bool has_power_profiles() {
        std::string out = exec("which powerprofilesctl 2>/dev/null");
        return !trim(out).empty();
    }

    bool is_performance() const { return _active_profile == "performance"; }
    bool is_balanced() const { return _active_profile == "balanced"; }
    bool is_power_saver() const { return _active_profile == "power-saver"; }

    void set_performance() { set_profile("performance"); }
    void set_balanced() { set_profile("balanced"); }
    void set_power_saver() { set_profile("power-saver"); }

    void cycle_profile() {
        refresh();
        if (_profiles.empty()) return;
        auto it = std::find(_profiles.begin(), _profiles.end(), _active_profile);
        if (it == _profiles.end() || ++it == _profiles.end()) {
            set_profile(_profiles[0]);
        } else {
            set_profile(*it);
        }
    }

    void start_polling(uint32_t interval_ms = 10000) {
        if (_poll_id > 0) return;
        _poll_id = g_timeout_add(interval_ms, [](gpointer data) -> gboolean {
            auto* self = static_cast<PowerProfilesService*>(data);
            std::string old = self->_active_profile;
            self->refresh();
            if (self->_active_profile != old) self->emit("profile-changed");
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
    void refresh() {
        _profiles.clear();

        std::string out = exec("powerprofilesctl list 2>/dev/null");
        std::istringstream stream(out);
        std::string line;
        while (std::getline(stream, line)) {
            line = trim(line);
            if (line.empty()) continue;

            // Lines with '*' are active profiles
            if (line.find('*') != std::string::npos) {
                // Extract profile name: "  * performance:" or "* performance:"
                size_t star = line.find('*');
                size_t colon = line.find(':', star);
                if (colon != std::string::npos) {
                    std::string name = trim(line.substr(star + 1, colon - star - 1));
                    _active_profile = name;
                    _profiles.push_back(name);
                }
            } else if (line.find(':') != std::string::npos && line.find("Driver:") == std::string::npos) {
                size_t colon = line.find(':');
                std::string name = trim(line.substr(0, colon));
                if (!name.empty()) {
                    _profiles.push_back(name);
                }
            }
        }

        // Fallback: try getting just the active profile
        if (_active_profile.empty()) {
            _active_profile = trim(exec("powerprofilesctl get 2>/dev/null"));
        }

        // Default profiles if none detected
        if (_profiles.empty()) {
            _profiles = {"power-saver", "balanced", "performance"};
        }
    }
};

PowerProfilesService* PowerProfilesService::_instance = nullptr;
