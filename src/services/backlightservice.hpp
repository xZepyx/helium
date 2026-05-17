#pragma once
#include <string>
#include <functional>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <dirent.h>
#include <glib.h>

class BacklightService {
private:
    static BacklightService* _instance;

    std::string _backlight_path;
    int32_t _max_brightness = 0;
    int32_t _brightness = 0;

    std::map<std::string, std::vector<std::function<void()>>> _signals;
    guint _poll_id = 0;

    BacklightService() {
        detect_backlight();
    }

    void detect_backlight() {
        const char* base = "/sys/class/backlight";
        DIR* dp = opendir(base);
        if (!dp) return;

        struct dirent* entry;
        while ((entry = readdir(dp)) != nullptr) {
            if (entry->d_name[0] == '.') continue;
            _backlight_path = std::string(base) + "/" + entry->d_name;
            break;
        }
        closedir(dp);

        if (!_backlight_path.empty()) {
            _max_brightness = read_int_from_file(_backlight_path + "/max_brightness");
            _brightness = read_int_from_file(_backlight_path + "/brightness");
        }
    }

    static int32_t read_int_from_file(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return 0;
        std::string line;
        std::getline(file, line);
        try { return std::stoi(line); } catch (...) { return 0; }
    }

    static void write_int_to_file(const std::string& path, int32_t value) {
        std::ofstream file(path);
        if (file.is_open()) {
            file << value;
        }
    }

public:
    static BacklightService& get_default() {
        if (!_instance) _instance = new BacklightService();
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

    int32_t get_brightness() {
        if (!_backlight_path.empty()) {
            _brightness = read_int_from_file(_backlight_path + "/actual_brightness");
            if (_brightness == 0) {
                _brightness = read_int_from_file(_backlight_path + "/brightness");
            }
        }
        return _brightness;
    }

    void set_brightness(int32_t value) {
        if (_backlight_path.empty()) return;

        if (value < 1) value = 1;
        if (value > _max_brightness) value = _max_brightness;

        write_int_to_file(_backlight_path + "/brightness", value);
        _brightness = value;
        emit("brightness-changed");
    }

    void set_brightness_percent(uint32_t percent) {
        if (_backlight_path.empty()) return;
        if (percent > 100) percent = 100;
        int32_t value = static_cast<int32_t>((percent * _max_brightness) / 100.0);
        if (value < 1) value = 1;
        set_brightness(value);
    }

    uint32_t get_brightness_percent() {
        if (_max_brightness == 0) return 0;
        get_brightness();
        return static_cast<uint32_t>((_brightness * 100.0) / _max_brightness);
    }

    int32_t get_max_brightness() {
        if (_max_brightness == 0 && !_backlight_path.empty()) {
            _max_brightness = read_int_from_file(_backlight_path + "/max_brightness");
        }
        return _max_brightness;
    }

    std::string get_backlight_device() const {
        return _backlight_path;
    }

    bool has_backlight() const {
        return !_backlight_path.empty();
    }

    std::vector<std::string> list_backlight_devices() {
        std::vector<std::string> devices;
        const char* base = "/sys/class/backlight";
        DIR* dp = opendir(base);
        if (!dp) return devices;
        struct dirent* entry;
        while ((entry = readdir(dp)) != nullptr) {
            if (entry->d_name[0] == '.') continue;
            devices.push_back(entry->d_name);
        }
        closedir(dp);
        return devices;
    }

    void start_polling(uint32_t interval_ms = 1000) {
        if (_poll_id > 0) return;
        _poll_id = g_timeout_add(interval_ms, [](gpointer data) -> gboolean {
            auto* self = static_cast<BacklightService*>(data);
            int32_t old = self->_brightness;
            self->get_brightness();
            if (self->_brightness != old) self->emit("brightness-changed");
            return TRUE;
        }, this);
    }

    void stop_polling() {
        if (_poll_id > 0) {
            g_source_remove(_poll_id);
            _poll_id = 0;
        }
    }
};

BacklightService* BacklightService::_instance = nullptr;
