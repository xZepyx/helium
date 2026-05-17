#pragma once
#include <gtk/gtk.h>
#include <glib.h>
#include <functional>

inline guint create_timeout(uint32_t ms, std::function<void()> callback) {
    auto cb = new std::function<void()>(callback);
    return g_timeout_add(ms, [](gpointer data) -> gboolean {
        auto fn = static_cast<std::function<void()>*>(data);
        (*fn)();
        delete fn;
        return FALSE;
    }, cb);
}

inline void cancel_timeout(guint id) {
    if (id > 0 && g_main_context_find_source_by_id(g_main_context_default(), id)) {
        g_source_remove(id);
    }
}
