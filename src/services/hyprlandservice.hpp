#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <algorithm>
#include <glib.h>
#include <fcntl.h>
#include "../compositors/hyprland/hyprland.hpp"

struct HyprlandWorkspace {
    int id = -1;
    std::string name;
    std::string monitor;
    int windows = 0;
    bool active = false;
    bool urgent = false;
};

struct HyprlandClient {
    std::string address;
    std::string class_name;
    std::string title;
    int workspace_id = -1;
    int x = 0, y = 0, w = 0, h = 0;
    bool floating = false;
    bool pinned = false;
    bool focused = false;
};

struct HyprlandMonitor {
    int id = -1;
    std::string name;
    int width = 0, height = 0;
    int x = 0, y = 0;
    float scale = 1.0f;
    bool primary = false;
    std::string active_workspace_name;
    int active_workspace_id = -1;
};

class HyprlandService : public Hyprland {
private:
    static HyprlandService* _instance;

    int _event_socket = -1;
    guint _event_source_id = 0;
    bool _listening = false;

    std::vector<HyprlandWorkspace> _workspaces;
    std::vector<HyprlandClient> _clients;
    std::vector<HyprlandMonitor> _monitors;
    std::string _active_window_class;
    std::string _active_window_title;
    std::string _active_window_address;
    int _active_workspace_id = -1;

    std::map<std::string, std::vector<std::function<void()>>> _signals;

    HyprlandService() {}

    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream stream(s);
        while (std::getline(stream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    static std::string json_string_value(const std::string& json, const std::string& key) {
        size_t key_pos = json.find("\"" + key + "\"");
        if (key_pos == std::string::npos) return "";
        size_t colon = json.find(':', key_pos);
        if (colon == std::string::npos) return "";
        size_t start = json.find_first_not_of(" \t", colon + 1);
        if (start == std::string::npos) return "";
        if (json[start] == '"') {
            size_t end = json.find('"', start + 1);
            if (end == std::string::npos) return "";
            return json.substr(start + 1, end - start - 1);
        }
        size_t end = json.find_first_of(",]}", start);
        if (end == std::string::npos) return json.substr(start);
        return json.substr(start, end - start);
    }

    static int json_int_value(const std::string& json, const std::string& key) {
        std::string val = json_string_value(json, key);
        if (val.empty()) return 0;
        try { return std::stoi(val); } catch (...) { return 0; }
    }

    static bool json_bool_value(const std::string& json, const std::string& key) {
        std::string val = json_string_value(json, key);
        return val == "true";
    }

    void connect_event_socket() {
        const char* xdg = getenv("XDG_RUNTIME_DIR");
        const char* sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");
        if (!xdg || !sig) return;

        std::string path = std::string(xdg) + "/hypr/" + sig + "/.socket2.sock";

        _event_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (_event_socket == -1) return;

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

        if (connect(_event_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            close(_event_socket);
            _event_socket = -1;
            return;
        }

        int flags = fcntl(_event_socket, F_GETFL, 0);
        fcntl(_event_socket, F_SETFL, flags | O_NONBLOCK);
    }

    static gboolean on_event_io(GIOChannel* source, GIOCondition condition, gpointer data) {
        auto* self = static_cast<HyprlandService*>(data);
        if (condition & G_IO_IN) {
            char buffer[4096];
            ssize_t n = read(self->_event_socket, buffer, sizeof(buffer) - 1);
            if (n > 0) {
                buffer[n] = '\0';
                self->parse_events(std::string(buffer, n));
            }
        }
        if (condition & G_IO_ERR || condition & G_IO_HUP) {
            self->_listening = false;
            return FALSE;
        }
        return TRUE;
    }

    void parse_events(const std::string& raw) {
        auto lines = split(raw, '\n');
        for (auto& line : lines) {
            size_t sep = line.find(">>");
            if (sep == std::string::npos) continue;
            std::string event = line.substr(0, sep);
            std::string data = line.substr(sep + 2);

            if (event == "workspace") {
                handle_workspace_event(data);
            } else if (event == "focusedmon") {
                handle_focusedmon_event(data);
            } else if (event == "activewindow") {
                handle_activewindow_event(data);
            } else if (event == "activewindowv2") {
                handle_activewindowv2_event(data);
            } else if (event == "openwindow") {
                handle_openwindow_event(data);
            } else if (event == "closewindow") {
                handle_closewindow_event(data);
            } else if (event == "movewindow") {
                handle_movewindow_event(data);
            } else if (event == "urgent") {
                handle_urgent_event(data);
            } else if (event == "monitoradded" || event == "monitorremoved") {
                refresh_monitors();
            } else if (event == "fullscreen") {
                emit("fullscreen");
            }

            emit(event);
        }
    }

    void handle_workspace_event(const std::string& data) {
        refresh_workspaces();
        for (auto& ws : _workspaces) {
            if (ws.name == data) {
                _active_workspace_id = ws.id;
                break;
            }
        }
        emit("workspace-changed");
    }

    void handle_focusedmon_event(const std::string& data) {
        refresh_workspaces();
        emit("focusedmon-changed");
    }

    void handle_activewindow_event(const std::string& data) {
        size_t comma = data.find(',');
        if (comma != std::string::npos) {
            _active_window_class = data.substr(0, comma);
            _active_window_title = data.substr(comma + 1);
        }
        emit("activewindow-changed");
    }

    void handle_activewindowv2_event(const std::string& data) {
        _active_window_address = data;
        refresh_clients();
        emit("activewindow-changed");
    }

    void handle_openwindow_event(const std::string& data) {
        refresh_clients();
        emit("window-opened");
    }

    void handle_closewindow_event(const std::string& data) {
        refresh_clients();
        emit("window-closed");
    }

    void handle_movewindow_event(const std::string& data) {
        refresh_clients();
        emit("window-moved");
    }

    void handle_urgent_event(const std::string& data) {
        for (auto& ws : _workspaces) {
            ws.urgent = (ws.name == data);
        }
        emit("urgent");
    }

    static size_t find_matching_close(const std::string& s, size_t open_pos, char open_ch, char close_ch) {
        if (s[open_pos] != open_ch) return std::string::npos;
        int depth = 1;
        for (size_t i = open_pos + 1; i < s.length(); i++) {
            if (s[i] == open_ch) depth++;
            else if (s[i] == close_ch) { depth--; if (depth == 0) return i; }
            else if (s[i] == '"') {
                i++;
                while (i < s.length() && s[i] != '"') {
                    if (s[i] == '\\') i++;
                    i++;
                }
            }
        }
        return std::string::npos;
    }

    static std::vector<std::map<std::string, std::string>> parse_json_array(const std::string& json) {
        std::vector<std::map<std::string, std::string>> result;
        size_t pos = 0;
        while (true) {
            size_t obj_start = json.find('{', pos);
            if (obj_start == std::string::npos) break;
            size_t obj_end = find_matching_close(json, obj_start, '{', '}');
            if (obj_end == std::string::npos) break;
            std::string obj = json.substr(obj_start, obj_end - obj_start + 1);
            pos = obj_end + 1;

            std::map<std::string, std::string> kv;
            size_t kpos = 0;
            while (true) {
                size_t kstart = obj.find('"', kpos);
                if (kstart == std::string::npos || kstart >= obj.length() - 1) break;
                size_t kend = obj.find('"', kstart + 1);
                if (kend == std::string::npos) break;
                std::string key = obj.substr(kstart + 1, kend - kstart - 1);

                size_t colon = obj.find(':', kend + 1);
                if (colon == std::string::npos) break;

                size_t vstart = obj.find_first_not_of(" \t", colon + 1);
                if (vstart == std::string::npos) break;

                std::string val;
                if (obj[vstart] == '"') {
                    size_t vend = obj.find('"', vstart + 1);
                    if (vend == std::string::npos) break;
                    val = obj.substr(vstart + 1, vend - vstart - 1);
                    kpos = vend + 1;
                } else if (obj[vstart] == '{') {
                    size_t vend = find_matching_close(obj, vstart, '{', '}');
                    if (vend == std::string::npos) break;
                    val = obj.substr(vstart, vend - vstart + 1);
                    kpos = vend + 1;
                } else if (obj[vstart] == '[') {
                    size_t vend = find_matching_close(obj, vstart, '[', ']');
                    if (vend == std::string::npos) break;
                    val = obj.substr(vstart, vend - vstart + 1);
                    kpos = vend + 1;
                } else {
                    size_t vend = obj.find_first_of(",}]", vstart);
                    if (vend == std::string::npos) {
                        val = obj.substr(vstart);
                        kpos = obj.length();
                    } else {
                        val = obj.substr(vstart, vend - vstart);
                        kpos = vend + 1;
                    }
                }
                kv[trim(key)] = trim(val);
            }
            result.push_back(kv);
        }
        return result;
    }

public:
    static HyprlandService& get_default() {
        if (!_instance) _instance = new HyprlandService();
        return *_instance;
    }

    void start_listening() {
        if (_listening) return;
        connect_event_socket();
        if (_event_socket == -1) return;

        GIOChannel* channel = g_io_channel_unix_new(_event_socket);
        _event_source_id = g_io_add_watch(channel, (GIOCondition)(G_IO_IN | G_IO_ERR | G_IO_HUP), on_event_io, this);
        g_io_channel_unref(channel);
        _listening = true;

        refresh_all();
    }

    void stop_listening() {
        if (!_listening) return;
        if (_event_source_id > 0) {
            g_source_remove(_event_source_id);
            _event_source_id = 0;
        }
        if (_event_socket != -1) {
            close(_event_socket);
            _event_socket = -1;
        }
        _listening = false;
    }

    bool is_listening() const { return _listening; }

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

    void refresh_all() {
        refresh_workspaces();
        refresh_clients();
        refresh_monitors();
        std::string json = send_command("activewindow");
        handle_activewindow_event(json_string_value(json, "class") + "," + json_string_value(json, "title"));
        _active_workspace_id = json_int_value(send_command("activeworkspace"), "id");
    }

    void refresh_workspaces() {
        _workspaces.clear();
        std::string json = send_command("workspaces");
        auto arr = parse_json_array(json);
        for (auto& item : arr) {
            HyprlandWorkspace ws;
            ws.id = std::stoi(item["id"]);
            ws.name = item["name"];
            ws.monitor = item["monitor"];
            ws.windows = std::stoi(item["windows"]);
            ws.active = item["lastwindow"] == item["lastwindow"]; // placeholder
            _workspaces.push_back(ws);
        }

        std::string active_json = send_command("activeworkspace");
        int active_id = json_int_value(active_json, "id");
        for (auto& ws : _workspaces) {
            ws.active = (ws.id == active_id);
        }
        _active_workspace_id = active_id;
    }

    // Extract value from a JSON object by key, handling nested objects
    static std::string extract_nested_value(const std::map<std::string, std::string>& obj, const std::string& dotted_key) {
        auto it = obj.find(dotted_key);
        if (it != obj.end()) return it->second;
        return "";
    }

    void refresh_clients() {
        _clients.clear();
        std::string json = send_command("clients");
        auto arr = parse_json_array(json);
        for (auto& item : arr) {
            HyprlandClient c;
            c.address = item["address"];
            c.class_name = item["class"];
            c.title = item["title"];

            std::string ws_id = extract_nested_value(item, "workspace.id");
            if (ws_id.empty()) ws_id = extract_nested_value(item, "workspace_id");
            c.workspace_id = ws_id.empty() ? 0 : std::stoi(ws_id);

            std::string at_str = extract_nested_value(item, "at");
            if (!at_str.empty() && at_str[0] == '[') {
                auto parts = split(at_str.substr(1, at_str.length() - 2), ',');
                if (parts.size() >= 1) c.x = std::stoi(trim(parts[0]));
                if (parts.size() >= 2) c.y = std::stoi(trim(parts[1]));
            }

            std::string size_str = extract_nested_value(item, "size");
            if (!size_str.empty() && size_str[0] == '[') {
                auto parts = split(size_str.substr(1, size_str.length() - 2), ',');
                if (parts.size() >= 1) c.w = std::stoi(trim(parts[0]));
                if (parts.size() >= 2) c.h = std::stoi(trim(parts[1]));
            }

            c.floating = item["floating"] == "true";
            c.pinned = item["pinned"] == "true";
            c.focused = item["focusHistoryID"] == "0";
            _clients.push_back(c);
        }
    }

    void refresh_monitors() {
        _monitors.clear();
        std::string json = send_command("monitors");
        auto arr = parse_json_array(json);
        for (auto& item : arr) {
            HyprlandMonitor m;
            m.id = std::stoi(item["id"]);
            m.name = item["name"];
            m.width = std::stoi(item["width"]);
            m.height = std::stoi(item["height"]);
            m.x = std::stoi(item["x"]);
            m.y = std::stoi(item["y"]);

            std::string scale_str = item["scale"];
            if (!scale_str.empty()) m.scale = std::stof(scale_str);

            m.primary = item["primary"] == "true";

            m.active_workspace_name = extract_nested_value(item, "activeWorkspace.name");
            std::string aw_id = extract_nested_value(item, "activeWorkspace.id");
            if (!aw_id.empty()) m.active_workspace_id = std::stoi(aw_id);

            _monitors.push_back(m);
        }
    }

    std::vector<HyprlandWorkspace> get_workspaces() {
        refresh_workspaces();
        return _workspaces;
    }

    HyprlandWorkspace get_active_workspace() {
        refresh_workspaces();
        for (auto& ws : _workspaces) {
            if (ws.active) return ws;
        }
        return HyprlandWorkspace();
    }

    std::vector<HyprlandClient> get_clients() {
        refresh_clients();
        return _clients;
    }

    std::vector<HyprlandMonitor> get_monitors() {
        refresh_monitors();
        return _monitors;
    }

    std::string dispatch(const std::string& cmd) {
        return send_command("dispatch " + cmd);
    }

    std::string keyword(const std::string& key, const std::string& value) {
        return send_command("keyword " + key + " " + value);
    }

    std::string get_active_window_class() const { return _active_window_class; }
    std::string get_active_window_title() const { return _active_window_title; }
    std::string get_active_window_address() const { return _active_window_address; }
    int get_active_workspace_id() const { return _active_workspace_id; }

    std::string batch(const std::vector<std::string>& commands) {
        std::string joined;
        for (size_t i = 0; i < commands.size(); i++) {
            if (i > 0) joined += "; ";
            joined += "j/" + commands[i];
        }
        return send_command("batch " + joined);
    }
};

HyprlandService* HyprlandService::_instance = nullptr;
