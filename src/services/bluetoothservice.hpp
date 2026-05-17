#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <cstdio>
#include <memory>
#include <array>
#include <sstream>
#include <glib.h>

struct BluetoothDevice {
    std::string mac;
    std::string name;
    bool connected = false;
    bool paired = false;
    bool trusted = false;
    int battery = -1;
    std::string icon;
    std::string type;
};

class BluetoothService {
private:
    static BluetoothService* _instance;

    std::vector<BluetoothDevice> _devices;
    std::map<std::string, std::vector<std::function<void()>>> _signals;
    guint _poll_id = 0;

    BluetoothService() {}

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

    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream stream(s);
        std::string token;
        while (std::getline(stream, token, delimiter)) {
            tokens.push_back(trim(token));
        }
        return tokens;
    }

public:
    static BluetoothService& get_default() {
        if (!_instance) _instance = new BluetoothService();
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

    std::vector<BluetoothDevice> get_devices() {
        refresh();
        return _devices;
    }

    bool is_connected(const std::string& mac = "") {
        refresh();
        if (mac.empty()) {
            for (auto& d : _devices) {
                if (d.connected) return true;
            }
            return false;
        }
        for (auto& d : _devices) {
            if (d.mac == mac) return d.connected;
        }
        return false;
    }

    int get_battery(const std::string& mac = "") {
        refresh();
        if (mac.empty()) {
            for (auto& d : _devices) {
                if (d.connected && d.battery >= 0) return d.battery;
            }
            return -1;
        }
        for (auto& d : _devices) {
            if (d.mac == mac) return d.battery;
        }
        return -1;
    }

    bool connect_device(const std::string& mac) {
        std::string out = exec("bluetoothctl connect " + mac + " 2>/dev/null");
        bool ok = (out.find("Connection successful") != std::string::npos);
        if (ok) {
            refresh();
            emit("device-connected");
        }
        return ok;
    }

    bool disconnect_device(const std::string& mac) {
        std::string out = exec("bluetoothctl disconnect " + mac + " 2>/dev/null");
        bool ok = (out.find("Successful disconnected") != std::string::npos ||
                   out.find("disconnected") != std::string::npos);
        if (ok) {
            refresh();
            emit("device-disconnected");
        }
        return ok;
    }

    bool pair_device(const std::string& mac) {
        std::string out = exec("bluetoothctl pair " + mac + " 2>/dev/null");
        bool ok = (out.find("Pairing successful") != std::string::npos);
        if (ok) {
            refresh();
            emit("device-paired");
        }
        return ok;
    }

    bool remove_device(const std::string& mac) {
        exec("bluetoothctl remove " + mac + " 2>/dev/null");
        refresh();
        emit("device-removed");
        return true;
    }

    void set_trust(const std::string& mac, bool trust) {
        if (trust) {
            exec("bluetoothctl trust " + mac + " 2>/dev/null");
        } else {
            exec("bluetoothctl untrust " + mac + " 2>/dev/null");
        }
        refresh();
    }

    void start_polling(uint32_t interval_ms = 5000) {
        if (_poll_id > 0) return;
        _poll_id = g_timeout_add(interval_ms, [](gpointer data) -> gboolean {
            auto* self = static_cast<BluetoothService*>(data);
            size_t old_count = self->_devices.size();
            self->refresh();
            if (self->_devices.size() != old_count) self->emit("devices-changed");
            return TRUE;
        }, this);
    }

    void stop_polling() {
        if (_poll_id > 0) {
            g_source_remove(_poll_id);
            _poll_id = 0;
        }
    }

    bool is_bluetooth_on() {
        std::string out = exec("bluetoothctl show 2>/dev/null | grep 'Powered:'");
        return (out.find("yes") != std::string::npos || out.find("on") != std::string::npos);
    }

    void set_bluetooth_on(bool on) {
        if (on) {
            exec("bluetoothctl power on 2>/dev/null");
        } else {
            exec("bluetoothctl power off 2>/dev/null");
        }
        emit("power-changed");
    }

    void toggle_bluetooth() {
        set_bluetooth_on(!is_bluetooth_on());
    }

    void start_scan() {
        exec("bluetoothctl scan on 2>/dev/null &");
    }

    void stop_scan() {
        exec("bluetoothctl scan off 2>/dev/null");
    }

private:
    void refresh() {
        _devices.clear();
        std::string out = exec("bluetoothctl devices 2>/dev/null");
        std::istringstream stream(out);
        std::string line;

        std::map<std::string, BluetoothDevice> device_map;

        while (std::getline(stream, line)) {
            line = trim(line);
            if (line.find("Device ") != 0) continue;
            auto parts = split(line, ' ');
            if (parts.size() < 3) continue;

            BluetoothDevice dev;
            dev.mac = parts[1];
            dev.name = parts[2];
            for (size_t i = 3; i < parts.size(); i++) dev.name += " " + parts[i];
            device_map[dev.mac] = dev;
        }

        std::string info = exec("bluetoothctl devices Connected 2>/dev/null");
        std::istringstream info_stream(info);
        while (std::getline(info_stream, line)) {
            auto parts = split(line, ' ');
            if (parts.size() >= 2 && device_map.count(parts[1])) {
                device_map[parts[1]].connected = true;
            }
        }

        std::string paired = exec("bluetoothctl devices Paired 2>/dev/null");
        std::istringstream paired_stream(paired);
        while (std::getline(paired_stream, line)) {
            auto parts = split(line, ' ');
            if (parts.size() >= 2 && device_map.count(parts[1])) {
                device_map[parts[1]].paired = true;
            }
        }

        std::string trusted = exec("bluetoothctl devices Trusted 2>/dev/null");
        std::istringstream trusted_stream(trusted);
        while (std::getline(trusted_stream, line)) {
            auto parts = split(line, ' ');
            if (parts.size() >= 2 && device_map.count(parts[1])) {
                device_map[parts[1]].trusted = true;
            }
        }

        for (auto& [mac, dev] : device_map) {
            std::string bat = exec("bluetoothctl info " + mac + " 2>/dev/null | grep 'Battery'");
            if (!bat.empty()) {
                size_t pct = bat.find('%');
                if (pct != std::string::npos) {
                    size_t start = pct;
                    while (start > 0 && isdigit(bat[start - 1])) start--;
                    dev.battery = std::stoi(bat.substr(start, pct - start));
                }
            }
            _devices.push_back(dev);
        }
    }
};

BluetoothService* BluetoothService::_instance = nullptr;
