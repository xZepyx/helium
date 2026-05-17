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

struct NetworkDevice {
    std::string name;
    std::string type;
    std::string state;
    bool connected = false;
};

struct NetworkConnection {
    std::string name;
    std::string uuid;
    std::string type;
    bool active = false;
    std::string device;
    int32_t strength = 0;
};

class NetworkService {
private:
    static NetworkService* _instance;

    std::string _ssid;
    int32_t _strength = 0;
    bool _connected = false;
    std::string _icon_name;
    std::string _device_name;

    std::map<std::string, std::vector<std::function<void()>>> _signals;
    guint _poll_id = 0;

    NetworkService() {}

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

    void determine_icon() {
        if (!_connected) {
            _icon_name = "network-wireless-offline-symbolic";
            return;
        }
        if (_strength >= 80) _icon_name = "network-wireless-signal-excellent-symbolic";
        else if (_strength >= 60) _icon_name = "network-wireless-signal-good-symbolic";
        else if (_strength >= 40) _icon_name = "network-wireless-signal-ok-symbolic";
        else if (_strength >= 20) _icon_name = "network-wireless-signal-weak-symbolic";
        else _icon_name = "network-wireless-signal-none-symbolic";
    }

public:
    static NetworkService& get_default() {
        if (!_instance) _instance = new NetworkService();
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

    std::string get_ssid() {
        refresh();
        return _ssid;
    }

    int32_t get_strength() {
        refresh();
        return _strength;
    }

    std::string get_icon_name() {
        refresh();
        return _icon_name;
    }

    bool is_connected() {
        refresh();
        return _connected;
    }

    std::string get_device_name() {
        refresh();
        return _device_name;
    }

    std::vector<NetworkDevice> get_devices() {
        std::string out = exec("nmcli -t device status 2>/dev/null");
        std::vector<NetworkDevice> devices;
        std::istringstream stream(out);
        std::string line;
        while (std::getline(stream, line)) {
            auto parts = split(line, ':');
            if (parts.size() >= 3) {
                NetworkDevice dev;
                dev.name = parts[0];
                dev.type = parts[1];
                dev.state = parts[2];
                dev.connected = (dev.state == "connected");
                devices.push_back(dev);
            }
        }
        return devices;
    }

    std::vector<NetworkConnection> get_connections() {
        std::string out = exec("nmcli -t connection show 2>/dev/null");
        std::vector<NetworkConnection> connections;
        std::istringstream stream(out);
        std::string line;
        while (std::getline(stream, line)) {
            auto parts = split(line, ':');
            if (parts.size() >= 4) {
                NetworkConnection conn;
                conn.name = parts[0];
                conn.uuid = parts[1];
                conn.type = parts[2];
                conn.device = parts[3];
                connections.push_back(conn);
            }
        }

        std::string active_out = exec("nmcli -t connection show --active 2>/dev/null");
        std::istringstream active_stream(active_out);
        while (std::getline(active_stream, line)) {
            auto parts = split(line, ':');
            if (parts.size() >= 1) {
                for (auto& conn : connections) {
                    if (conn.name == parts[0]) {
                        conn.active = true;
                        break;
                    }
                }
            }
        }

        return connections;
    }

    std::vector<std::string> scan_wifi() {
        std::string out = exec("nmcli -t device wifi list 2>/dev/null | head -20");
        std::vector<std::string> networks;
        std::istringstream stream(out);
        std::string line;
        while (std::getline(stream, line)) {
            auto parts = split(line, ':');
            if (parts.size() >= 2) {
                networks.push_back(parts[0]); // SSID
            }
        }
        return networks;
    }

    void connect_to_network(const std::string& ssid, const std::string& password = "") {
        std::string cmd = "nmcli device wifi connect \"" + ssid + "\"";
        if (!password.empty()) cmd += " password \"" + password + "\"";
        exec(cmd + " 2>/dev/null");
        emit("connection-changed");
    }

    void disconnect() {
        if (!_device_name.empty()) {
            exec("nmcli device disconnect " + _device_name + " 2>/dev/null");
            emit("connection-changed");
        }
    }

    void enable_wifi() {
        exec("nmcli radio wifi on 2>/dev/null");
        emit("wifi-changed");
    }

    void disable_wifi() {
        exec("nmcli radio wifi off 2>/dev/null");
        emit("wifi-changed");
    }

    bool is_wifi_enabled() {
        std::string out = exec("nmcli radio wifi 2>/dev/null");
        return trim(out) == "enabled";
    }

    void enable_networking() {
        exec("nmcli networking on 2>/dev/null");
        emit("networking-changed");
    }

    void disable_networking() {
        exec("nmcli networking off 2>/dev/null");
        emit("networking-changed");
    }

    bool is_networking_enabled() {
        std::string out = exec("nmcli networking 2>/dev/null");
        return trim(out) == "enabled";
    }

    void start_polling(uint32_t interval_ms = 5000) {
        if (_poll_id > 0) return;
        _poll_id = g_timeout_add(interval_ms, [](gpointer data) -> gboolean {
            auto* self = static_cast<NetworkService*>(data);
            bool old_connected = self->_connected;
            std::string old_ssid = self->_ssid;
            int32_t old_strength = self->_strength;
            self->refresh();
            if (self->_connected != old_connected) self->emit("connection-changed");
            if (self->_ssid != old_ssid) self->emit("ssid-changed");
            if (self->_strength != old_strength) self->emit("strength-changed");
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
        std::string out = exec("nmcli -t device wifi show 2>/dev/null");

        _connected = false;
        _ssid.clear();
        _strength = 0;

        std::istringstream stream(out);
        std::string line;
        while (std::getline(stream, line)) {
            size_t colon = line.find(':');
            if (colon == std::string::npos) continue;
            std::string key = trim(line.substr(0, colon));
            std::string val = trim(line.substr(colon + 1));

            if (key == "SSID") {
                _ssid = val;
                _connected = !val.empty() && val != "--";
            } else if (key == "CHANNEL") {
                // channel info
            }
        }

        // Try to get signal strength from nmcli
        if (_connected && !_ssid.empty()) {
            std::string sig_out = exec("nmcli -f SSID,SIGNAL -t device wifi list 2>/dev/null | grep -F \"" + _ssid + "\"");
            if (!sig_out.empty()) {
                auto parts = split(sig_out, ':');
                if (parts.size() >= 2) {
                    try { _strength = std::stoi(parts[1]); } catch (...) {}
                }
            }

            // Get device name
            std::string dev_out = exec("nmcli -t device status 2>/dev/null | grep ':connected'");
            if (!dev_out.empty()) {
                auto parts = split(dev_out, ':');
                if (!parts.empty()) _device_name = parts[0];
            }
        }

        determine_icon();
    }
};

NetworkService* NetworkService::_instance = nullptr;
