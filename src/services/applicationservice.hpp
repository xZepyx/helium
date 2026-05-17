#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <cstdio>
#include <memory>
#include <array>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <glib.h>

struct Application {
    std::string id;
    std::string name;
    std::string generic_name;
    std::string comment;
    std::string icon;
    std::string exec;
    std::string path;
    std::vector<std::string> categories;
    std::vector<std::string> mime_types;
    bool no_display = false;
    bool terminal = false;
    bool startup = false;
};

class ApplicationService {
private:
    static ApplicationService* _instance;

    std::vector<Application> _applications;
    std::map<std::string, std::string> _icon_cache;
    bool _loaded = false;

    std::map<std::string, std::vector<std::function<void()>>> _signals;

    ApplicationService() {}

    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

    static std::vector<std::string> get_data_dirs() {
        std::vector<std::string> dirs;
        const char* xdg_data = getenv("XDG_DATA_DIRS");
        if (xdg_data) {
            std::istringstream stream(xdg_data);
            std::string dir;
            while (std::getline(stream, dir, ':')) {
                dirs.push_back(trim(dir) + "/applications");
            }
        } else {
            dirs.push_back("/usr/share/applications");
            dirs.push_back("/usr/local/share/applications");
        }
        const char* home = getenv("HOME");
        if (home) {
            dirs.push_back(std::string(home) + "/.local/share/applications");
        }
        return dirs;
    }

    static bool dir_exists(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    static std::string get_desktop_entry(const std::string& content, const std::string& key) {
        std::istringstream stream(content);
        std::string line;
        bool in_desktop = false;
        while (std::getline(stream, line)) {
            line = trim(line);
            if (line == "[Desktop Entry]") { in_desktop = true; continue; }
            if (line.find('[') == 0 && line.find(']') != std::string::npos) { in_desktop = false; continue; }
            if (!in_desktop) continue;
            if (line.find(key + "=") == 0) {
                return line.substr(key.length() + 1);
            }
        }
        return "";
    }

    static std::string get_desktop_entry_localized(const std::string& content, const std::string& key) {
        std::string val = get_desktop_entry(content, key);
        if (!val.empty()) return val;

        const char* lang = getenv("LANG");
        if (lang) {
            std::string lang_str(lang);
            size_t dot = lang_str.find('.');
            if (dot != std::string::npos) lang_str = lang_str.substr(0, dot);

            std::string localized_key = key + "[" + lang_str + "]";
            val = get_desktop_entry(content, localized_key);
            if (!val.empty()) return val;

            size_t underscore = lang_str.find('_');
            if (underscore != std::string::npos) {
                localized_key = key + "[" + lang_str.substr(0, underscore) + "]";
                val = get_desktop_entry(content, localized_key);
            }
        }
        return val;
    }

    Application parse_desktop_file(const std::string& filepath) {
        Application app;
        app.path = filepath;

        std::ifstream file(filepath);
        if (!file.is_open()) return app;

        std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

        app.id = filepath.substr(filepath.find_last_of('/') + 1);
        if (app.id.size() > 8 && app.id.substr(app.id.size() - 8) == ".desktop") {
            app.id = app.id.substr(0, app.id.size() - 8);
        }

        app.name = get_desktop_entry_localized(content, "Name");
        app.generic_name = get_desktop_entry_localized(content, "GenericName");
        app.comment = get_desktop_entry_localized(content, "Comment");
        app.icon = get_desktop_entry(content, "Icon");
        app.exec = get_desktop_entry(content, "Exec");

        std::string nodisplay = get_desktop_entry(content, "NoDisplay");
        app.no_display = (nodisplay == "true");

        std::string term = get_desktop_entry(content, "Terminal");
        app.terminal = (term == "true");

        std::string startup = get_desktop_entry(content, "StartupNotify");
        app.startup = (startup == "true");

        std::string cats = get_desktop_entry(content, "Categories");
        if (!cats.empty()) {
            std::istringstream cstream(cats);
            std::string cat;
            while (std::getline(cstream, cat, ';')) {
                std::string c = trim(cat);
                if (!c.empty()) app.categories.push_back(c);
            }
        }

        std::string mimes = get_desktop_entry(content, "MimeType");
        if (!mimes.empty()) {
            std::istringstream mstream(mimes);
            std::string mime;
            while (std::getline(mstream, mime, ';')) {
                std::string m = trim(mime);
                if (!m.empty()) app.mime_types.push_back(m);
            }
        }

        return app;
    }

public:
    static ApplicationService& get_default() {
        if (!_instance) _instance = new ApplicationService();
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

    std::vector<Application> get_applications() {
        if (!_loaded) load_applications();
        return _applications;
    }

    std::string get_icon(const std::string& app_id) {
        if (_icon_cache.count(app_id)) return _icon_cache[app_id];

        if (!_loaded) load_applications();

        for (auto& app : _applications) {
            if (app.id == app_id) {
                find_icon_path(const_cast<Application&>(app));
                break;
            }
        }

        if (_icon_cache.count(app_id)) return _icon_cache[app_id];
        return "";
    }

    std::string get_name(const std::string& app_id) {
        if (!_loaded) load_applications();
        for (auto& app : _applications) {
            if (app.id == app_id) return app.name;
        }
        return app_id;
    }

    void launch(const std::string& app_id) {
        for (auto& app : _applications) {
            if (app.id == app_id) {
                launch(app);
                return;
            }
        }
    }

    void launch(const Application& app) {
        std::string cmd = app.exec;

        // Replace %f, %F, %u, %U, %i, %c, %k
        size_t pos;
        while ((pos = cmd.find("%f")) != std::string::npos) cmd.replace(pos, 2, "");
        while ((pos = cmd.find("%F")) != std::string::npos) cmd.replace(pos, 2, "");
        while ((pos = cmd.find("%u")) != std::string::npos) cmd.replace(pos, 2, "");
        while ((pos = cmd.find("%U")) != std::string::npos) cmd.replace(pos, 2, "");

        pos = cmd.find("%i");
        if (pos != std::string::npos) {
            std::string icon_flag;
            if (!app.icon.empty()) icon_flag = "--icon " + app.icon;
            cmd.replace(pos, 2, icon_flag);
        }

        pos = cmd.find("%c");
        if (pos != std::string::npos) cmd.replace(pos, 2, app.name);

        pos = cmd.find("%k");
        if (pos != std::string::npos) cmd.replace(pos, 2, app.path);

        if (app.terminal) {
            cmd = "x-terminal-emulator -e " + cmd;
        }

        cmd += " &";
        system(cmd.c_str());
        emit("app-launched");
    }

    std::vector<Application> search(const std::string& query) {
        if (!_loaded) load_applications();

        std::vector<Application> results;
        std::string lower_query = query;
        std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);

        for (auto& app : _applications) {
            if (app.no_display) continue;

            std::string lower_name = app.name;
            std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

            std::string lower_generic = app.generic_name;
            std::transform(lower_generic.begin(), lower_generic.end(), lower_generic.begin(), ::tolower);

            if (lower_name.find(lower_query) != std::string::npos ||
                lower_generic.find(lower_query) != std::string::npos) {
                results.push_back(app);
            }
        }

        return results;
    }

    std::vector<Application> get_by_category(const std::string& category) {
        if (!_loaded) load_applications();

        std::vector<Application> results;
        std::string lower_cat = category;
        std::transform(lower_cat.begin(), lower_cat.end(), lower_cat.begin(), ::tolower);

        for (auto& app : _applications) {
            if (app.no_display) continue;
            for (auto& c : app.categories) {
                std::string lower_c = c;
                std::transform(lower_c.begin(), lower_c.end(), lower_c.begin(), ::tolower);
                if (lower_c == lower_cat) {
                    results.push_back(app);
                    break;
                }
            }
        }
        return results;
    }

    void reload() {
        _applications.clear();
        _icon_cache.clear();
        _loaded = false;
        load_applications();
        emit("applications-reloaded");
    }

private:
    void load_applications() {
        _applications.clear();
        auto dirs = get_data_dirs();

        for (auto& dir : dirs) {
            if (!dir_exists(dir)) continue;

            DIR* dp = opendir(dir.c_str());
            if (!dp) continue;

            struct dirent* entry;
            while ((entry = readdir(dp)) != nullptr) {
                std::string name(entry->d_name);
                if (name.size() <= 8 || name.substr(name.size() - 8) != ".desktop") continue;
                if (name[0] == '.') continue;

                std::string filepath = dir + "/" + name;
                Application app = parse_desktop_file(filepath);

                if (!app.name.empty() && !app.exec.empty()) {
                    // Check for duplicate IDs
                    bool dup = false;
                    for (auto& existing : _applications) {
                        if (existing.id == app.id) {
                            dup = true;
                            break;
                        }
                    }
                    if (!dup) {
                        find_icon_path(app);
                        _applications.push_back(app);
                    }
                }
            }
            closedir(dp);
        }

        _loaded = true;
    }

    void find_icon_path(Application& app) {
        if (app.icon.empty()) return;
        if (app.icon[0] == '/') {
            _icon_cache[app.id] = app.icon;
            return;
        }

        std::vector<std::string> icon_dirs;
        const char* home = getenv("HOME");
        if (home) {
            icon_dirs.push_back(std::string(home) + "/.local/share/icons");
        }
        icon_dirs.push_back("/usr/share/icons");
        icon_dirs.push_back("/usr/share/pixmaps");

        std::vector<std::string> themes = {"hicolor", "Adwaita", "gnome", "Papirus", "breeze"};
        std::vector<std::string> sizes = {"scalable", "48x48", "64x64",  "32x32", "24x24", "16x16"};
        std::vector<std::string> contexts = {"apps", "places", "categories", "status", "devices"};
        std::vector<std::string> exts = {".png", ".svg", ".xpm"};

        for (auto& base : icon_dirs) {
            for (auto& theme : themes) {
                for (auto& context : contexts) {
                    for (auto& size : sizes) {
                        for (auto& ext : exts) {
                            std::string path = base + "/" + theme + "/" + size + "/" + context + "/" + app.icon + ext;
                            if (access(path.c_str(), F_OK) == 0) {
                                _icon_cache[app.id] = path;
                                return;
                            }
                        }
                    }
                }
            }
            // Direct icon lookup
            for (auto& ext : exts) {
                std::string path = base + "/" + app.icon + ext;
                if (access(path.c_str(), F_OK) == 0) {
                    _icon_cache[app.id] = path;
                    return;
                }
            }
        }

        std::string pix_path = "/usr/share/pixmaps/" + app.icon + ".png";
        if (access(pix_path.c_str(), F_OK) == 0) {
            _icon_cache[app.id] = pix_path;
            return;
        }
        pix_path = "/usr/share/pixmaps/" + app.icon + ".xpm";
        if (access(pix_path.c_str(), F_OK) == 0) {
            _icon_cache[app.id] = pix_path;
            return;
        }
    }
};

ApplicationService* ApplicationService::_instance = nullptr;
