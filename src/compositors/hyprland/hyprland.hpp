#pragma once
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

class Hyprland {
public:
    static std::string send_command(std::string cmd) {
        const char* xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
        const char* hypr_sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");

        if (!xdg_runtime_dir || !hypr_sig) return "Error: Envs not set";

        std::string socket_path = std::string(xdg_runtime_dir) + "/hypr/" + hypr_sig + "/.socket.sock";

        int sock = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            close(sock);
            return "Error: Connection failed";
        }

        // Hyprland expects commands starting with '/' or 'j/' for JSON
        std::string formatted_cmd = "j/" + cmd; 
        send(sock, formatted_cmd.c_str(), formatted_cmd.length(), 0);

        char buffer[8192];
        std::string response = "";
        ssize_t n;
        while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            response.append(buffer, n);
        }

        close(sock);
        return response;
    }
};