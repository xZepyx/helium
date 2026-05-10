#include <gtk/gtk.h>

#include "types/panel.hpp"
#include "managers/cssmanager.hpp"
#include "types/label.hpp"

int main(int argc, char** argv) {
    gtk_init();

    CSS_MANAGER::load("gradle/style.css"); // gradle is just a directory that stores these test files.

    Panel panel({
        .namespace_ = "example_window",
        .monitor = 0,
        .anchor = {
            "top",
            "right",
            "left"
        },
        .exclusive = true,
        .layer = "top",
        .kb_mode = "none",
        .popup = false
    });

    panel.add_css_class("panel");
    panel.show();

    panel.set_height(50);

    Label label("Hello World");
    panel.set_child(&label);
    GMainLoop* loop =
        g_main_loop_new(nullptr, false);

    g_main_loop_run(loop);

    return 0;
}