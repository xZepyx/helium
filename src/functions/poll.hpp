#include <gtk/gtk.h>
#include <glib.h>
#include <pybind11/functional.h>

void create_poll(uint32_t ms, std::function<bool()> callback) {
    // Move the callback to the heap so the GLib C-closure can access it
    auto wrapper = new std::function<bool()>(callback);

    g_timeout_add(ms, [](gpointer data) -> gboolean {
        auto cb = static_cast<std::function<bool()>*>(data);
        
        // Execute the Python function
        bool keep_running = (*cb)();
        
        if (!keep_running) {
            delete cb; // Cleanup heap memory
            return FALSE;
        }
        return TRUE;
    }, wrapper);
}