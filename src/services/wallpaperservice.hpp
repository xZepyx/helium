#pragma once
#include <string>
#include <functional>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <array>
#include <sstream>
#include <glib.h>

class WallpaperService {
private:
    static WallpaperService* _instance;

    std::string _current_wallpaper;
    std::string _backend; // "swww", "hyprctl", "swaybg", "feh"

    std::map<std::string, std::vector<std::function<void()>>> _signals;

    WallpaperService() {
        detect_backend();
    }

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

    void detect_backend() {
        std::string which = exec("which swww 2>/dev/null");
        if (!trim(which).empty()) {
            _backend = "swww";
            return;
        }

        which = exec("which swaybg 2>/dev/null");
        if (!trim(which).empty()) {
            _backend = "swaybg";
            return;
        }

        which = exec("which hyprctl 2>/dev/null");
        if (!trim(which).empty()) {
            _backend = "hyprctl";
            return;
        }

        which = exec("which feh 2>/dev/null");
        if (!trim(which).empty()) {
            _backend = "feh";
            return;
        }

        _backend = "none";
    }

public:
    static WallpaperService& get_default() {
        if (!_instance) _instance = new WallpaperService();
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

    void set_wallpaper(const std::string& path) {
        if (!file_exists(path)) return;

        std::string abs_path = resolve_path(path);

        if (_backend == "swww") {
            exec("swww img \"" + abs_path + "\" --transition-type any 2>/dev/null");
        } else if (_backend == "hyprctl") {
            exec("hyprctl hyprpaper wallpaper \",\"" + abs_path + "\" 2>/dev/null");
        } else if (_backend == "feh") {
            exec("feh --bg-fill \"" + abs_path + "\" 2>/dev/null");
        } else if (_backend == "swaybg") {
            // swaybg is typically launched as a process, notify via pkill + restart
            exec("pkill swaybg 2>/dev/null; swaybg -i \"" + abs_path + "\" &");
        }

        _current_wallpaper = abs_path;
        emit("wallpaper-changed");
    }

    std::string get_wallpaper() const {
        return _current_wallpaper;
    }

    std::string get_backend() const {
        return _backend;
    }

    bool has_backend() const {
        return _backend != "none";
    }

    void set_wallpaper_for_monitor(const std::string& monitor, const std::string& path) {
        if (!file_exists(path)) return;
        std::string abs_path = resolve_path(path);

        if (_backend == "swww") {
            exec("swww img -o \"" + monitor + "\" \"" + abs_path + "\" --transition-type any 2>/dev/null");
        } else if (_backend == "hyprctl") {
            exec("hyprctl hyprpaper wallpaper \"" + monitor + "\",\"" + abs_path + "\" 2>/dev/null");
        } else {
            set_wallpaper(path);
        }

        _current_wallpaper = abs_path;
        emit("wallpaper-changed");
    }

    void restore_wallpaper() {
        if (!_current_wallpaper.empty()) {
            set_wallpaper(_current_wallpaper);
        }
    }

private:
    static bool file_exists(const std::string& path) {
        return access(path.c_str(), F_OK) == 0;
    }

    static std::string resolve_path(const std::string& path) {
        char* resolved = ::realpath(path.c_str(), nullptr);
        if (resolved) {
            std::string result(resolved);
            free(resolved);
            return result;
        }
        return path;
    }
};

WallpaperService* WallpaperService::_instance = nullptr;
