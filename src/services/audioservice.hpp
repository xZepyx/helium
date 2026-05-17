#pragma once
#include <string>
#include <functional>
#include <map>
#include <cstdio>
#include <memory>
#include <sstream>
#include <array>
#include <glib.h>

struct AudioSink {
    std::string name;
    std::string description;
    uint32_t volume = 0;
    bool muted = false;
    bool active = false;
};

class AudioService {
private:
    static AudioService* _instance;

    AudioSink _default_sink;
    bool _has_pipewire = false;
    bool _has_pulse = false;

    std::map<std::string, std::vector<std::function<void()>>> _signals;
    guint _poll_id = 0;

    AudioService() {
        detect_backend();
    }

    void detect_backend() {
        _has_pipewire = (system("pactl info 2>/dev/null | grep -q 'PipeWire'") == 0);
        _has_pulse = (system("pactl info 2>/dev/null | grep -q 'PulseAudio'") == 0);
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

    void parse_default_sink_from_pactl(const std::string& output) {
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line)) {
            line = trim(line);
            if (line.find("Volume:") != std::string::npos) {
                size_t pct = line.find('%');
                if (pct != std::string::npos) {
                    size_t num_start = pct;
                    while (num_start > 0 && isdigit(line[num_start - 1])) num_start--;
                    _default_sink.volume = std::stoi(line.substr(num_start, pct - num_start));
                }
            } else if (line.find("Mute:") != std::string::npos) {
                _default_sink.muted = (line.find("yes") != std::string::npos);
            } else if (line.find("Name:") != std::string::npos) {
                _default_sink.name = trim(line.substr(5));
            } else if (line.find("Description:") != std::string::npos) {
                _default_sink.description = trim(line.substr(12));
            }
        }
    }

public:
    static AudioService& get_default() {
        if (!_instance) _instance = new AudioService();
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

    uint32_t get_volume() {
        refresh();
        return _default_sink.volume;
    }

    void set_volume(uint32_t percent) {
        if (percent > 100) percent = 100;
        exec("pactl set-sink-volume @DEFAULT_SINK@ " + std::to_string(percent) + "%");
        _default_sink.volume = percent;
        emit("volume-changed");
    }

    bool get_muted() {
        refresh();
        return _default_sink.muted;
    }

    void set_muted(bool muted) {
        if (muted) {
            exec("pactl set-sink-mute @DEFAULT_SINK@ 1");
        } else {
            exec("pactl set-sink-mute @DEFAULT_SINK@ 0");
        }
        _default_sink.muted = muted;
        emit("muted-changed");
    }

    void toggle_muted() {
        set_muted(!_default_sink.muted);
    }

    std::string get_default_sink_name() {
        refresh();
        return _default_sink.name;
    }

    std::string get_default_sink_description() {
        refresh();
        return _default_sink.description;
    }

    AudioSink get_default_sink() {
        refresh();
        return _default_sink;
    }

    std::vector<std::string> list_sinks() {
        std::string out = exec("pactl list sinks short");
        std::vector<std::string> sinks;
        std::istringstream stream(out);
        std::string line;
        while (std::getline(stream, line)) {
            auto parts = split(trim(line), '\t');
            if (parts.size() >= 2) sinks.push_back(parts[1]);
        }
        return sinks;
    }

    bool has_pipewire() const { return _has_pipewire; }
    bool has_pulse() const { return _has_pulse; }

    void start_polling(uint32_t interval_ms = 1000) {
        if (_poll_id > 0) return;
        _poll_id = g_timeout_add(interval_ms, [](gpointer data) -> gboolean {
            auto* self = static_cast<AudioService*>(data);
            uint32_t old_vol = self->_default_sink.volume;
            bool old_muted = self->_default_sink.muted;
            self->refresh();
            if (self->_default_sink.volume != old_vol) self->emit("volume-changed");
            if (self->_default_sink.muted != old_muted) self->emit("muted-changed");
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
        std::string out = exec("pactl get-sink-volume @DEFAULT_SINK@ 2>/dev/null; "
                                "pactl get-sink-mute @DEFAULT_SINK@ 2>/dev/null");
        parse_default_sink_from_pactl(out);

        std::string info = exec("pactl info 2>/dev/null | grep 'Default Sink'");
        size_t colon = info.find(':');
        if (colon != std::string::npos) {
            _default_sink.name = trim(info.substr(colon + 1));
        }
    }

    static std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream stream(s);
        std::string token;
        while (std::getline(stream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
};

AudioService* AudioService::_instance = nullptr;
