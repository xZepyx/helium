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

struct MprisPlayer {
    std::string name;
    std::string identity;
    std::string artist;
    std::string title;
    std::string album;
    std::string art_url;
    std::string status; // Playing, Paused, Stopped
    int64_t position = 0;
    int64_t length = 0;
    double volume = 0.0;
    bool can_play = true;
    bool can_pause = true;
    bool can_go_next = true;
    bool can_go_previous = true;
    bool shuffle = false;
    std::string loop_status; // None, Track, Playlist
};

class MprisService {
private:
    static MprisService* _instance;

    std::vector<MprisPlayer> _players;
    std::map<std::string, std::vector<std::function<void()>>> _signals;
    guint _poll_id = 0;

    MprisService() {}

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

    static std::string playerctl(const std::string& args, const std::string& player = "") {
        std::string cmd = "playerctl";
        if (!player.empty()) cmd += " --player=" + player;
        cmd += " " + args + " 2>/dev/null";
        return trim(exec(cmd));
    }

    MprisPlayer query_player(const std::string& name) {
        MprisPlayer p;
        p.name = name;
        p.identity = playerctl("--player=" + name + " metadata --format '{{ identity }}'");
        p.artist = playerctl("--player=" + name + " metadata --format '{{ artist }}'");
        p.title = playerctl("--player=" + name + " metadata --format '{{ title }}'");
        p.album = playerctl("--player=" + name + " metadata --format '{{ album }}'");
        p.art_url = playerctl("--player=" + name + " metadata --format '{{ mpris:artUrl }}'");
        p.status = playerctl("--player=" + name + " status");

        std::string pos_str = playerctl("--player=" + name + " position");
        if (!pos_str.empty()) {
            try { p.position = static_cast<int64_t>(std::stod(pos_str) * 1000000); } catch (...) {}
        }

        std::string len_str = playerctl("--player=" + name + " metadata --format '{{ mpris:length }}'");
        if (!len_str.empty()) {
            try { p.length = std::stoll(len_str); } catch (...) {}
        }

        return p;
    }

public:
    static MprisService& get_default() {
        if (!_instance) _instance = new MprisService();
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

    std::vector<std::string> get_players() {
        std::string out = exec("playerctl -l 2>/dev/null");
        std::vector<std::string> players;
        std::istringstream stream(out);
        std::string line;
        while (std::getline(stream, line)) {
            std::string name = trim(line);
            if (!name.empty()) players.push_back(name);
        }
        return players;
    }

    std::string get_artist(const std::string& player = "") {
        return playerctl("metadata --format '{{ artist }}'", player);
    }

    std::string get_title(const std::string& player = "") {
        return playerctl("metadata --format '{{ title }}'", player);
    }

    std::string get_status(const std::string& player = "") {
        return playerctl("status", player);
    }

    std::string get_album(const std::string& player = "") {
        return playerctl("metadata --format '{{ album }}'", player);
    }

    std::string get_art_url(const std::string& player = "") {
        return playerctl("metadata --format '{{ mpris:artUrl }}'", player);
    }

    int64_t get_position(const std::string& player = "") {
        std::string pos = playerctl("position", player);
        if (pos.empty()) return 0;
        try { return static_cast<int64_t>(std::stod(pos) * 1000000); } catch (...) { return 0; }
    }

    int64_t get_length(const std::string& player = "") {
        std::string len = playerctl("metadata --format '{{ mpris:length }}'", player);
        if (len.empty()) return 0;
        try { return std::stoll(len); } catch (...) { return 0; }
    }

    std::vector<MprisPlayer> get_all_players() {
        refresh();
        return _players;
    }

    MprisPlayer get_player(const std::string& name) {
        return query_player(name);
    }

    MprisPlayer get_active_player() {
        auto players = get_players();
        if (players.empty()) return MprisPlayer();
        return query_player(players[0]);
    }

    void play(const std::string& player = "") {
        playerctl("play", player);
        emit("player-changed");
    }

    void pause(const std::string& player = "") {
        playerctl("pause", player);
        emit("player-changed");
    }

    void play_pause(const std::string& player = "") {
        playerctl("play-pause", player);
        emit("player-changed");
    }

    void next(const std::string& player = "") {
        playerctl("next", player);
        emit("player-changed");
    }

    void previous(const std::string& player = "") {
        playerctl("previous", player);
        emit("player-changed");
    }

    void stop(const std::string& player = "") {
        playerctl("stop", player);
        emit("player-changed");
    }

    void set_position(int64_t position_us, const std::string& player = "") {
        double pos_sec = position_us / 1000000.0;
        playerctl("position " + std::to_string(pos_sec), player);
        emit("player-changed");
    }

    void set_volume(double vol, const std::string& player = "") {
        playerctl("volume " + std::to_string(vol), player);
        emit("player-changed");
    }

    void set_shuffle(bool shuffle, const std::string& player = "") {
        playerctl(std::string("shuffle ") + (shuffle ? "on" : "off"), player);
        emit("player-changed");
    }

    void set_loop(const std::string& mode, const std::string& player = "") {
        playerctl("loop " + mode, player);
        emit("player-changed");
    }

    void open(const std::string& url, const std::string& player = "") {
        playerctl("open " + url, player);
        emit("player-changed");
    }

    void start_polling(uint32_t interval_ms = 2000) {
        if (_poll_id > 0) return;
        _poll_id = g_timeout_add(interval_ms, [](gpointer data) -> gboolean {
            static_cast<MprisService*>(data)->refresh();
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
        _players.clear();
        auto names = get_players();
        for (auto& name : names) {
            _players.push_back(query_player(name));
        }
        emit("players-changed");
    }
};

MprisService* MprisService::_instance = nullptr;
