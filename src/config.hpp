#pragma once
#include <string>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <glib.h>

using json = nlohmann::json;

class Config {
private:
    json _data;
    json _defaults;
    std::string _path;
    guint _write_source = 0;
    bool _dirty = false;

    static Config* _instance;

    Config() = default;

public:
    ~Config() {
        if (_write_source) g_source_remove(_write_source);
        if (_dirty) save();
    }

private:

    void merge_recursive(json& target, const json& defaults) {
        for (auto it = defaults.begin(); it != defaults.end(); ++it) {
            if (it->is_object() && target.contains(it.key()) && target[it.key()].is_object()) {
                merge_recursive(target[it.key()], it.value());
            } else if (!target.contains(it.key())) {
                target[it.key()] = it.value();
            }
        }
    }

    static gboolean delayed_write(gpointer user_data) {
        auto self = static_cast<Config*>(user_data);
        self->save();
        self->_write_source = 0;
        return G_SOURCE_REMOVE;
    }

public:
    static Config& get_default() {
        if (!_instance) _instance = new Config();
        return *_instance;
    }

    static void destroy() {
        delete _instance;
        _instance = nullptr;
    }

    void set_defaults(const json& defaults) {
        _defaults = defaults;
        merge_recursive(_data, _defaults);
        if (!_path.empty()) save();
    }

    std::string path() const { return _path; }

    void set_path(const std::string& p) {
        _path = p;
        load();
    }

    void load() {
        if (_path.empty()) return;

        std::ifstream file(_path);
        if (!file.good()) {
            save();
            return;
        }

        try {
            _data = json::parse(file);
            merge_recursive(_data, _defaults);
            save();
        } catch (...) {
            _data = _defaults;
            save();
        }
    }

    void save() {
        if (_path.empty()) return;

        try {
            auto dir = std::filesystem::path(_path).parent_path();
            if (!dir.empty() && !std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
            }

            std::ofstream file(_path);
            file << _data.dump(2);
            file.close();
        } catch (...) {
            g_warning("Config: failed to save to %s", _path.c_str());
        }

        _dirty = false;
    }

    void schedule_save() {
        _dirty = true;
        if (_write_source) g_source_remove(_write_source);
        _write_source = g_timeout_add(200, delayed_write, this);
    }

    void reload() {
        load();
    }

    // Typed access with dot notation
    template<typename T>
    T get(const std::string& key, T default_val = T()) const {
        try {
            json j = _data;
            size_t pos = 0, end;
            while ((end = key.find('.', pos)) != std::string::npos) {
                j = j[key.substr(pos, end - pos)];
                pos = end + 1;
            }
            return j[key.substr(pos)].get<T>();
        } catch (...) {
            return default_val;
        }
    }

    std::string get_string(const std::string& key, const std::string& default_val = "") const {
        return get<std::string>(key, default_val);
    }

    int get_int(const std::string& key, int default_val = 0) const {
        return get<int>(key, default_val);
    }

    double get_double(const std::string& key, double default_val = 0.0) const {
        return get<double>(key, default_val);
    }

    bool get_bool(const std::string& key, bool default_val = false) const {
        return get<bool>(key, default_val);
    }

    template<typename T>
    void set(const std::string& key, T val) {
        try {
            json* j = &_data;
            size_t pos = 0, end;
            while ((end = key.find('.', pos)) != std::string::npos) {
                auto segment = key.substr(pos, end - pos);
                if (!j->contains(segment) || !(*j)[segment].is_object()) {
                    (*j)[segment] = json::object();
                }
                j = &(*j)[segment];
                pos = end + 1;
            }
            (*j)[key.substr(pos)] = val;
            schedule_save();
        } catch (...) {}
    }

    // Raw json access
    json& data() { return _data; }
    const json& data() const { return _data; }

    // Check if config file exists
    bool exists() const {
        return !_path.empty() && std::filesystem::exists(_path);
    }

    // List all top-level keys
    std::vector<std::string> keys() const {
        std::vector<std::string> k;
        for (auto it = _data.begin(); it != _data.end(); ++it) {
            k.push_back(it.key());
        }
        return k;
    }
};

Config* Config::_instance = nullptr;
