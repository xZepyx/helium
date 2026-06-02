#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/embed.h>
#include <gtk/gtk.h>

#include "../types/widget.hpp"
#include "../types/label.hpp"
#include "../types/button.hpp"
#include "../types/panel.hpp"
#include "../types/box.hpp"
#include "../types/centerbox.hpp"
#include "../types/image.hpp"
#include "../types/eventbox.hpp"
#include "../types/entry.hpp"
#include "../types/materialsymbol.hpp"
#include "../types/overlay.hpp"
#include "../types/scrolledwindow.hpp"
#include "../types/window.hpp"
#include "../types/regularwindow.hpp"
#include "../types/revealerwindow.hpp"
#include "../types/arrow.hpp"
#include "../types/arrowbutton.hpp"
#include "../types/calendar.hpp"
#include "../types/checkbutton.hpp"
#include "../types/corner.hpp"
#include "../types/dropdown.hpp"
#include "../types/fixed.hpp"
#include "../types/grid.hpp"
#include "../types/headerbar.hpp"
#include "../types/icon.hpp"
#include "../types/picture.hpp"
#include "../types/revealer.hpp"
#include "../types/scale.hpp"
#include "../types/separator.hpp"
#include "../types/spinbutton.hpp"
#include "../types/stack.hpp"
#include "../types/stackpage.hpp"
#include "../types/stackswitcher.hpp"
#include "../types/switch.hpp"
#include "../types/togglebutton.hpp"
#include "../types/listbox.hpp"
#include "../types/listboxrow.hpp"
#include "../types/popovermenu.hpp"

#include "../managers/cssmanager.hpp"
#include "../managers/windowmanager.hpp"
#include "../managers/iconmanager.hpp"
#include "../functions/poll.hpp"
#include "../functions/timeout.hpp"
#include "../functions/filemonitor.hpp"
#include "../compositors/hyprland/hyprland.hpp"
#include "../services/hyprlandservice.hpp"
#include "../services/audioservice.hpp"
#include "../services/notificationservice.hpp"
#include "../services/bluetoothservice.hpp"
#include "../services/networkservice.hpp"
#include "../services/mprisservice.hpp"
#include "../services/backlightservice.hpp"
#include "../services/wallpaperservice.hpp"
#include "../services/powerprofilesservice.hpp"
#include "../services/applicationservice.hpp"
#include "../services/systemtrayservice.hpp"
#include "../config.hpp"

namespace py = pybind11;

namespace {

// Recursively convert a Python class instance to nested dicts
py::object _cfg_convert(const py::object& val) {
    auto isinstance_fn = py::module_::import("builtins").attr("isinstance");
    auto dict_type = py::module_::import("builtins").attr("dict");

    if (isinstance_fn(val, dict_type).cast<bool>()
        || py::isinstance<py::list>(val) || py::isinstance<py::tuple>(val)
        || py::isinstance<py::str>(val) || py::isinstance<py::bool_>(val)
        || py::isinstance<py::int_>(val) || py::isinstance<py::float_>(val)
        || val.is_none())
        return val;

    if (py::hasattr(val, "__dict__")) {
        py::dict result;
        auto d = py::cast<py::dict>(py::module_::import("builtins").attr("vars")(val));
        for (const auto& item : d) {
            result[item.first] = _cfg_convert(
                py::reinterpret_borrow<py::object>(item.second));
        }
        return std::move(result);
    }
    return val;
}

} // anonymous namespace

// --- hot-reload helpers ---
static std::vector<GFileMonitor*>& py_monitors() {
    static std::vector<GFileMonitor*> monitors;
    return monitors;
}

static void on_py_changed(GFileMonitor*, GFile* file, GFile*,
                           GFileMonitorEvent event, gpointer) {
    if (event != G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT
        && event != G_FILE_MONITOR_EVENT_CREATED)
        return;

    char* cpath = g_file_get_path(file);
    if (!cpath) return;
    std::string path(cpath);
    g_free(cpath);

    if (path.size() < 3
        || path.substr(path.size() - 3) != ".py")
        return;

    try {
        py::gil_scoped_acquire gil;
        py::exec(R"(
import os, sys
os.execl(sys.executable, sys.executable, *sys.argv)
)");
    } catch (py::error_already_set& e) {
        g_printerr("[Helium] Reload error: %s\n", e.what());
    } catch (std::exception& e) {
        g_printerr("[Helium] Reload error: %s\n", e.what());
    }
}

static void watch_py_dir(const char* dir, std::vector<GFileMonitor*>& out) {
    GDir* d = g_dir_open(dir, 0, nullptr);
    if (!d) return;
    const char* name;
    while ((name = g_dir_read_name(d)) != nullptr) {
        if (name[0] == '.') continue;
        std::string full = std::string(dir) + "/" + name;
        GFile* f = g_file_new_for_path(full.c_str());
        if (g_file_query_file_type(f, G_FILE_QUERY_INFO_NONE, nullptr)
            == G_FILE_TYPE_DIRECTORY) {
            watch_py_dir(full.c_str(), out);
        } else if (strstr(name, ".py")) {
            GFileMonitor* mon = g_file_monitor_file(f, G_FILE_MONITOR_NONE, nullptr, nullptr);
            if (mon) {
                g_signal_connect(mon, "changed", G_CALLBACK(on_py_changed), nullptr);
                out.push_back(mon);
            }
        }
        g_object_unref(f);
    }
    g_dir_close(d);
}

// Helper: recursively convert json to pybind11 objects
static py::object json_to_py(const json& j) {
    if (j.is_null()) return py::none();
    if (j.is_boolean()) return py::bool_(j.get<bool>());
    if (j.is_number_integer()) return py::int_(j.get<int>());
    if (j.is_number_float()) return py::float_(j.get<double>());
    if (j.is_string()) return py::str(j.get<std::string>());
    if (j.is_array()) {
        py::list list;
        for (const auto& item : j) list.append(json_to_py(item));
        return std::move(list);
    }
    if (j.is_object()) {
        py::dict dict;
        for (auto it = j.begin(); it != j.end(); ++it)
            dict[py::str(it.key())] = json_to_py(it.value());
        return std::move(dict);
    }
    return py::none();
}

// Helper: convert a pybind11 object to json
static json py_to_json(const py::handle& obj) {
    if (obj.is_none()) return nullptr;
    if (py::isinstance<py::bool_>(obj)) return obj.cast<bool>();
    if (py::isinstance<py::int_>(obj)) return obj.cast<int>();
    if (py::isinstance<py::float_>(obj)) return obj.cast<double>();
    if (py::isinstance<py::str>(obj)) return obj.cast<std::string>();
    if (py::isinstance<py::list>(obj) || py::isinstance<py::tuple>(obj)) {
        json arr = json::array();
        for (const auto& item : obj) arr.push_back(py_to_json(item));
        return arr;
    }
    if (py::isinstance<py::dict>(obj)) {
        json dict = json::object();
        py::dict d = py::reinterpret_borrow<py::dict>(obj);
        for (auto item : d) {
            dict[py::str(item.first).cast<std::string>()] = py_to_json(item.second);
        }
        return dict;
    }
    return nullptr;
}

PYBIND11_MODULE(helium, m) {
    m.doc() = "Helium shell framework - ported from Ignis to C++";

    m.def("init", []() {
        gtk_init();

        const char* dd = g_getenv("HELIUM_DAEMON_DIR");
        if (dd) {
            auto& mons = py_monitors();
            watch_py_dir(dd, mons);
        }
    });

    m.def("reload", []() {
        py::gil_scoped_acquire gil;
        py::exec(R"(
import os, sys
os.execl(sys.executable, sys.executable, *sys.argv)
)");
    });
    m.def("run", []() {
        GMainLoop* loop = g_main_loop_new(nullptr, false);
        g_main_loop_run(loop);
    });
    m.def("load_css", [](const std::string& path) {
        CSS_MANAGER::load(path.c_str());
    }, py::arg("path"));
    m.def("load_css_string", [](const std::string& css, const std::string& priority) {
        StylePriority p = StylePriority::Application;
        if (priority == "user") p = StylePriority::User;
        else if (priority == "theme") p = StylePriority::Theme;
        else if (priority == "fallback") p = StylePriority::Fallback;
        CSS_MANAGER::load_string(css, p);
    }, py::arg("css"), py::arg("priority") = "application");
    m.def("reset_css", []() { CSS_MANAGER::reset(); });
    m.def("reload_css", []() { CSS_MANAGER::reload(); });

    m.def("n_monitors", []() {
        auto display = gdk_display_get_default();
        return display ? g_list_model_get_n_items(gdk_display_get_monitors(display)) : 0;
    });





    // Config singleton
    py::class_<Config> config_cls(m, "Config");
    config_cls
        .def_static("get_default", &Config::get_default, py::return_value_policy::reference)
        .def("set_path", &Config::set_path, py::arg("path"))
        .def("path", &Config::path)
        .def("load", &Config::load)
        .def("save", &Config::save)
        .def("reload", &Config::reload)
        .def("merge_defaults", [](Config& self, const py::dict& data) {
            self.set_defaults(py_to_json(data));
        }, py::arg("data"))
        .def("get", [](Config& self, const std::string& key, const py::object& default_val) {
            const json* j = &self.data();
            size_t pos = 0, end;
            while ((end = key.find('.', pos)) != std::string::npos) {
                auto segment = key.substr(pos, end - pos);
                if (!j->is_object() || !j->contains(segment))
                    return default_val;
                j = &j->at(segment);
                pos = end + 1;
            }
            auto last = key.substr(pos);
            if (!j->is_object() || !j->contains(last))
                return default_val;
            return json_to_py(j->at(last));
        }, py::arg("key"), py::arg("default") = py::none())
        .def("set", [](Config& self, const std::string& key, const py::object& value) {
            auto segments = key;
            json* j = &self.data();
            size_t pos = 0, end;
            while ((end = segments.find('.', pos)) != std::string::npos) {
                auto segment = segments.substr(pos, end - pos);
                if (!j->contains(segment) || !(*j)[segment].is_object()) {
                    (*j)[segment] = json::object();
                }
                j = &(*j)[segment];
                pos = end + 1;
            }
            (*j)[segments.substr(pos)] = py_to_json(value);
            self.schedule_save();
        }, py::arg("key"), py::arg("value"))
        .def("keys", &Config::keys)
        .def("exists", &Config::exists)
        .def_property_readonly("data", [](Config& self) {
            return json_to_py(self.data());
        })
        .def("__repr__", [](Config& self) {
            return "<Config path='" + self.path() + "'>";
        });

    m.attr("config") = py::cast(&Config::get_default(), py::return_value_policy::reference);

    m.def("get_monitor_geometry", [](int monitor) {
        auto display = gdk_display_get_default();
        if (!display) return std::vector<int>{0, 0, 0, 0};
        auto monitors = gdk_display_get_monitors(display);
        int n = g_list_model_get_n_items(monitors);
        if (monitor < 0 || monitor >= n) return std::vector<int>{0, 0, 0, 0};
        auto gdk_mon = GDK_MONITOR(g_list_model_get_item(monitors, monitor));
        if (!gdk_mon) return std::vector<int>{0, 0, 0, 0};
        GdkRectangle geom;
        gdk_monitor_get_geometry(gdk_mon, &geom);
        return std::vector<int>{geom.x, geom.y, geom.width, geom.height};
    }, py::arg("monitor"));

    // helium.functions submodule
    py::module_ f = m.def_submodule("functions", "Helium functional utilities");
    f.def("Poll", &create_poll, py::arg("ms"), py::arg("callback"));
    f.def("Timeout", [](uint32_t ms, std::function<void()> callback) {
        return create_timeout(ms, callback);
    }, py::arg("ms"), py::arg("callback"));
    f.def("CancelTimeout", &cancel_timeout, py::arg("id"));
    f.def("watch_file", [](const std::string& path, std::function<void(std::string, std::string)> callback) {
        FileMonitor::watch(path, callback);
    }, py::arg("path"), py::arg("callback"));
    f.def("unwatch_file", &FileMonitor::unwatch, py::arg("path"));
    f.def("config_from_class", [](const py::object& obj) {
        return _cfg_convert(obj);
    }, py::arg("obj"),
    "Convert a class instance to a nested dict for config.merge_defaults()");

    // helium.types submodule
    py::module_ t = m.def_submodule("types", "Helium widget types");

    // compositor stuff
    py::module_ comp = m.def_submodule("compositor", "Compositor specific APIs");
    py::module_ hypr = comp.def_submodule("hyprland", "Hyprland IPC bindings");

    hypr.def("hyprland_dispatch", &Hyprland::send_command,
             "Send a command to Hyprland IPC", py::arg("command"));

    hypr.def("get_workspaces", []() {
        return Hyprland::send_command("workspaces");
    }, "Get list of workspaces");

    hypr.def("get_active_workspace", []() {
        return Hyprland::send_command("activeworkspace");
    }, "Get the currently focused workspace");

    hypr.def("get_clients", []() {
        return Hyprland::send_command("clients");
    }, "Get list of all open windows");

    hypr.def("get_monitors", []() {
        return Hyprland::send_command("monitors");
    }, "Get list of active monitors");

    // Base Widget
    py::class_<Widget>(t, "Widget")
        .def("show", &Widget::show)
        .def("hide", [](Widget& w) { gtk_widget_set_visible(w.get_native(), FALSE); })
        .def("add_css_class", &Widget::add_css_class)
        .def("remove_css_class", &Widget::remove_css_class)
        .def("set_style", &Widget::set_style, py::arg("css_style"))
        .def("connect", &Widget::connect_signal, py::arg("signal_name"), py::arg("callback"))
        .def("set_hexpand", [](Widget& w, bool expand) { gtk_widget_set_hexpand(w.get_native(), expand); })
        .def("set_vexpand", [](Widget& w, bool expand) { gtk_widget_set_vexpand(w.get_native(), expand); })
        .def("get_hexpand", [](Widget& w) { return gtk_widget_get_hexpand(w.get_native()); })
        .def("get_vexpand", [](Widget& w) { return gtk_widget_get_vexpand(w.get_native()); })
        .def("set_halign", [](Widget& w, const std::string& align) {
            GtkAlign a = GTK_ALIGN_FILL;
            if (align == "start") a = GTK_ALIGN_START;
            else if (align == "center") a = GTK_ALIGN_CENTER;
            else if (align == "end") a = GTK_ALIGN_END;
            gtk_widget_set_halign(w.get_native(), a);
        })
        .def("set_valign", [](Widget& w, const std::string& align) {
            GtkAlign a = GTK_ALIGN_FILL;
            if (align == "start") a = GTK_ALIGN_START;
            else if (align == "center") a = GTK_ALIGN_CENTER;
            else if (align == "end") a = GTK_ALIGN_END;
            gtk_widget_set_valign(w.get_native(), a);
        })
        .def("set_size_request", [](Widget& w, int width, int height) {
            gtk_widget_set_size_request(w.get_native(), width, height);
        })
        .def("set_name", [](Widget& w, const std::string& name) {
            gtk_widget_set_name(w.get_native(), name.c_str());
        })
        .def("get_name", [](Widget& w) {
            return std::string(gtk_widget_get_name(w.get_native()));
        })
        .def("set_sensitive", [](Widget& w, bool sensitive) {
            gtk_widget_set_sensitive(w.get_native(), sensitive);
        })
        .def("get_sensitive", [](Widget& w) {
            return gtk_widget_get_sensitive(w.get_native());
        })
        .def("get_visible", [](Widget& w) {
            return gtk_widget_get_visible(w.get_native());
        });

    // Label
    py::class_<Label, Widget>(t, "Label")
        .def(py::init<const std::string&>(), py::arg("label") = "")
        .def("set_label", &Label::set_label)
        .def("get_label", &Label::get_label)
        .def("set_use_markup", [](Label& l, bool v) { gtk_label_set_use_markup(GTK_LABEL(l.get_native()), v); })
        .def("set_wrap", [](Label& l, bool v) { gtk_label_set_wrap(GTK_LABEL(l.get_native()), v); })
        .def("set_justify", [](Label& l, const std::string& j) {
            GtkJustification jst = GTK_JUSTIFY_LEFT;
            if (j == "center") jst = GTK_JUSTIFY_CENTER;
            else if (j == "right") jst = GTK_JUSTIFY_RIGHT;
            else if (j == "fill") jst = GTK_JUSTIFY_FILL;
            gtk_label_set_justify(GTK_LABEL(l.get_native()), jst);
        })
        .def("set_ellipsize", [](Label& l, const std::string& e) {
            PangoEllipsizeMode em = PANGO_ELLIPSIZE_NONE;
            if (e == "start") em = PANGO_ELLIPSIZE_START;
            else if (e == "middle") em = PANGO_ELLIPSIZE_MIDDLE;
            else if (e == "end") em = PANGO_ELLIPSIZE_END;
            gtk_label_set_ellipsize(GTK_LABEL(l.get_native()), em);
        })
        .def("set_max_width_chars", [](Label& l, int v) { gtk_label_set_max_width_chars(GTK_LABEL(l.get_native()), v); })
        .def("set_xalign", [](Label& l, float v) { gtk_label_set_xalign(GTK_LABEL(l.get_native()), v); });

    // MaterialSymbol
    py::class_<MaterialSymbol, Label>(t, "MaterialSymbol")
        .def(py::init<std::string, float, bool>(),
            py::arg("symbol"),
            py::arg("size") = 24.0,
            py::arg("fill") = false);

    // Button
    py::class_<Button, Widget>(t, "Button")
        .def(py::init([](std::string label) {
            ButtonProperties props;
            props.label = label;
            return new Button(props);
        }), py::arg("label") = "")
        .def("on_click", &Button::on_click, py::arg("callback"))
        .def("set_label", [](Button& b, const std::string& label) {
            gtk_button_set_label(GTK_BUTTON(b.get_native()), label.c_str());
        })
        .def("get_label", [](Button& b) {
            return std::string(gtk_button_get_label(GTK_BUTTON(b.get_native())));
        });

    // ToggleButton
    py::class_<ToggleButton, Widget>(t, "ToggleButton")
        .def(py::init<const std::string&, bool>(), py::arg("label") = "", py::arg("active") = false)
        .def("on_toggled", &ToggleButton::on_toggled)
        .def("on_click", &ToggleButton::on_click)
        .def("set_active", &ToggleButton::set_active)
        .def("get_active", &ToggleButton::get_active)
        .def("set_label", &ToggleButton::set_label)
        .def("get_label", &ToggleButton::get_label);

    // CheckButton
    py::class_<CheckButton, Widget>(t, "CheckButton")
        .def(py::init<const std::string&, bool>(), py::arg("label") = "", py::arg("active") = false)
        .def("on_toggled", &CheckButton::on_toggled)
        .def("set_active", &CheckButton::set_active)
        .def("get_active", &CheckButton::get_active)
        .def("set_label", &CheckButton::set_label)
        .def("get_label", &CheckButton::get_label)
        .def("set_group", &CheckButton::set_group);

    // Switch
    py::class_<Switch, Widget>(t, "Switch")
        .def(py::init<bool>(), py::arg("active") = false)
        .def("on_change", &Switch::on_change)
        .def("set_active", &Switch::set_active)
        .def("get_active", &Switch::get_active);

    // Scale
    py::class_<Scale, Widget>(t, "Scale")
        .def(py::init<double, double, double, double, std::string>(),
             py::arg("min") = 0, py::arg("max") = 100, py::arg("step") = 1,
             py::arg("value") = 0, py::arg("orientation") = "horizontal")
        .def("on_change", &Scale::on_change)
        .def("set_value", &Scale::set_value)
        .def("get_value", &Scale::get_value)
        .def("set_min", &Scale::set_min)
        .def("get_min", &Scale::get_min)
        .def("set_max", &Scale::set_max)
        .def("get_max", &Scale::get_max)
        .def("set_step", &Scale::set_step)
        .def("get_step", &Scale::get_step)
        .def("set_digits", &Scale::set_digits);

    // SpinButton
    py::class_<SpinButton, Widget>(t, "SpinButton")
        .def(py::init<double, double, double, double>(),
             py::arg("min") = 0, py::arg("max") = 100, py::arg("step") = 1, py::arg("value") = 0)
        .def("on_change", &SpinButton::on_change)
        .def("set_value", &SpinButton::set_value)
        .def("get_value", &SpinButton::get_value)
        .def("set_min", &SpinButton::set_min)
        .def("get_min", &SpinButton::get_min)
        .def("set_max", &SpinButton::set_max)
        .def("get_max", &SpinButton::get_max)
        .def("set_step", &SpinButton::set_step)
        .def("get_step", &SpinButton::get_step)
        .def("set_digits", &SpinButton::set_digits)
        .def("set_wrap", &SpinButton::set_wrap)
        .def("get_value_as_int", &SpinButton::get_value_as_int);

    // Entry
    py::class_<Entry, Widget>(t, "Entry")
        .def(py::init<>())
        .def("set_text", &Entry::set_text)
        .def("get_text", &Entry::get_text)
        .def("on_activate", &Entry::on_activate)
        .def("on_change", &Entry::on_change)
        .def("set_placeholder_text", [](Entry& e, const std::string& text) {
            gtk_entry_set_placeholder_text(GTK_ENTRY(e.get_native()), text.c_str());
        });

    // Box
    py::class_<Box, Widget>(t, "Box")
        .def(py::init([](std::string orientation, int spacing, std::vector<Widget*> children, std::string halign, std::string valign, bool hexpand, bool vexpand) {
            BoxProperties props{orientation, spacing, children, halign, valign, hexpand, vexpand};
            return new Box(props);
        }),
        py::arg("orientation") = "horizontal",
        py::arg("spacing") = 0,
        py::arg("children") = std::vector<Widget*>{},
        py::arg("halign") = "fill",
        py::arg("valign") = "fill",
        py::arg("hexpand") = false,
        py::arg("vexpand") = false)
        .def("add", &Box::add)
        .def("clear", &Box::clear)
        .def("append", &Box::append)
        .def("remove", &Box::remove)
        .def("prepend", &Box::prepend);

    // CenterBox
    py::class_<CenterBox, Widget>(t, "CenterBox")
        .def(py::init<>())
        .def("set_start", &CenterBox::set_start)
        .def("set_center", &CenterBox::set_center)
        .def("set_end", &CenterBox::set_end);

    // Grid
    py::class_<Grid, Widget>(t, "Grid")
        .def(py::init([](int cols, int rows) {
            GridProperties props;
            return new Grid(props);
        }), py::arg("column_num") = 0, py::arg("row_num") = 0)
        .def("attach", &Grid::attach, py::arg("child"), py::arg("column"), py::arg("row"), py::arg("width") = 1, py::arg("height") = 1)
        .def("remove", &Grid::remove)
        .def("set_column_homogeneous", &Grid::set_column_homogeneous)
        .def("set_row_homogeneous", &Grid::set_row_homogeneous)
        .def("set_column_spacing", &Grid::set_column_spacing)
        .def("set_row_spacing", &Grid::set_row_spacing);

    // Fixed
    py::class_<Fixed, Widget>(t, "Fixed")
        .def(py::init<>())
        .def("put", &Fixed::put)
        .def("remove", &Fixed::remove)
        .def("move", &Fixed::move);

    // Overlay
    py::class_<Overlay, Widget>(t, "Overlay")
        .def(py::init<>())
        .def("set_child", &Overlay::set_child)
        .def("add_overlay", &Overlay::add_overlay);

    // Revealer
    py::class_<Revealer, Widget>(t, "Revealer")
        .def(py::init<>())
        .def("set_child", &Revealer::set_child)
        .def("set_reveal_child", &Revealer::set_reveal_child)
        .def("get_reveal_child", &Revealer::get_reveal_child)
        .def("set_transition_duration", &Revealer::set_transition_duration)
        .def("set_transition_type", &Revealer::set_transition_type)
        .def("get_transition_type", &Revealer::get_transition_type)
        .def("toggle", &Revealer::toggle);

    // ScrolledWindow (also available as Scroll)
    py::class_<ScrolledWindow, Widget>(t, "ScrolledWindow")
        .def(py::init<>())
        .def("set_child", &ScrolledWindow::set_child)
        .def("set_policy", &ScrolledWindow::set_policy);

    // Stack
    py::class_<Stack, Widget>(t, "Stack")
        .def(py::init<>())
        .def("add_child", &Stack::add_child)
        .def("remove", &Stack::remove)
        .def("set_visible_child", &Stack::set_visible_child)
        .def("set_visible_child_name", &Stack::set_visible_child_name)
        .def("get_visible_child_name", &Stack::get_visible_child_name)
        .def("set_transition_type", &Stack::set_transition_type)
        .def("set_transition_duration", &Stack::set_transition_duration)
        .def("set_hhomogeneous", &Stack::set_hhomogeneous)
        .def("set_vhomogeneous", &Stack::set_vhomogeneous);

    // StackSwitcher
    py::class_<StackSwitcher, Widget>(t, "StackSwitcher")
        .def(py::init<>())
        .def("set_stack", &StackSwitcher::set_stack)
        .def("get_stack", &StackSwitcher::get_stack);

    // Separator
    py::class_<Separator, Widget>(t, "Separator")
        .def(py::init<const std::string&>(), py::arg("orientation") = "horizontal");

    // Calendar
    py::class_<Calendar, Widget>(t, "Calendar")
        .def(py::init<>())
        .def("select_day", &Calendar::select_day)
        .def("select_month", &Calendar::select_month)
        .def("mark_day", &Calendar::mark_day)
        .def("unmark_day", &Calendar::unmark_day)
        .def("clear_marks", &Calendar::clear_marks)
        .def("set_show_heading", &Calendar::set_show_heading)
        .def("set_show_day_names", &Calendar::set_show_day_names)
        .def("set_show_week_numbers", &Calendar::set_show_week_numbers)
        .def("on_day_selected", &Calendar::on_day_selected)
        .def("on_month_changed", &Calendar::on_month_changed);

    // DropDown
    py::class_<DropDown, Widget>(t, "DropDown")
        .def(py::init<const std::vector<std::string>&, int>(),
             py::arg("items") = std::vector<std::string>{}, py::arg("selected") = 0)
        .def("set_items", &DropDown::set_items)
        .def("get_items", &DropDown::get_items)
        .def("set_selected", &DropDown::set_selected)
        .def("get_selected", &DropDown::get_selected)
        .def("get_selected_item", &DropDown::get_selected_item)
        .def("on_selected", &DropDown::on_selected);

    // EventBox
    py::class_<EventBox, Widget>(t, "EventBox")
        .def(py::init<>())
        .def("set_child", &EventBox::set_child)
        .def("on_hover", &EventBox::on_hover)
        .def("on_click", &EventBox::on_click)
        .def("on_right_click", &EventBox::on_right_click)
        .def("on_middle_click", &EventBox::on_middle_click)
        .def("on_scroll_up", &EventBox::on_scroll_up)
        .def("on_scroll_down", &EventBox::on_scroll_down);

    // Image
    py::class_<Image, Widget>(t, "Image")
        .def_static("from_icon", &Image::from_icon, py::arg("name"))
        .def_static("from_file", &Image::from_file, py::arg("path"));

    // Icon
    py::class_<Icon, Widget>(t, "Icon")
        .def(py::init<int>(), py::arg("pixel_size") = -1)
        .def("set_icon_name", &Icon::set_icon_name)
        .def("get_icon_name", &Icon::get_icon_name)
        .def("set_from_file", &Icon::set_from_file)
        .def("set_pixel_size", &Icon::set_pixel_size)
        .def("get_pixel_size", &Icon::get_pixel_size)
        .def_static("from_icon_name", &Icon::from_icon_name, py::arg("name"), py::arg("size") = -1)
        .def_static("from_file", &Icon::from_file, py::arg("path"), py::arg("size") = -1);

    // Picture
    py::class_<Picture, Widget>(t, "Picture")
        .def(py::init<const std::string&>(), py::arg("file_path") = "")
        .def("set_filename", &Picture::set_filename)
        .def("get_filename", &Picture::get_filename)
        .def("set_keep_aspect_ratio", &Picture::set_keep_aspect_ratio)
        .def("get_keep_aspect_ratio", &Picture::get_keep_aspect_ratio)
        .def("set_can_shrink", &Picture::set_can_shrink)
        .def("get_can_shrink", &Picture::get_can_shrink);

    // Corner
    py::class_<Corner, Widget>(t, "Corner")
        .def(py::init<int, int, const std::string&, const std::string&>(),
             py::arg("size") = 20, py::arg("radius") = 10,
             py::arg("corner") = "top-left", py::arg("color") = "black")
        .def("set_size", &Corner::set_size)
        .def("get_size", &Corner::get_size)
        .def("set_radius", &Corner::set_radius)
        .def("get_radius", &Corner::get_radius)
        .def("set_corner", &Corner::set_corner)
        .def("get_corner", &Corner::get_corner)
        .def("set_color", &Corner::set_color)
        .def("get_color", &Corner::get_color);

    // ArrowDirection enum (must be registered before Arrow)
    py::enum_<ArrowDirection>(t, "ArrowDirection")
        .value("UP", ArrowDirection::UP)
        .value("DOWN", ArrowDirection::DOWN)
        .value("LEFT", ArrowDirection::LEFT)
        .value("RIGHT", ArrowDirection::RIGHT);

    // Arrow
    py::class_<Arrow, Widget>(t, "Arrow")
        .def(py::init<ArrowDirection, int>(),
             py::arg("direction") = ArrowDirection::UP, py::arg("pixel_size") = 24)
        .def("set_rotated", &Arrow::set_rotated)
        .def("get_rotated", &Arrow::get_rotated)
        .def("set_degree", &Arrow::set_degree)
        .def("get_degree", &Arrow::get_degree)
        .def("set_time", &Arrow::set_time)
        .def("get_time", &Arrow::get_time)
        .def("set_direction", &Arrow::set_direction)
        .def("get_direction", &Arrow::get_direction)
        .def("set_counterclockwise", &Arrow::set_counterclockwise)
        .def("get_counterclockwise", &Arrow::get_counterclockwise)
        .def("toggle", &Arrow::toggle);

    // ArrowButton
    py::class_<ArrowButton, Button>(t, "ArrowButton")
        .def(py::init<Arrow*, const std::string&>(), py::arg("arrow"), py::arg("label") = "")
        .def("toggle", &ArrowButton::toggle);

    // HeaderBar
    py::class_<HeaderBar, Widget>(t, "HeaderBar")
        .def(py::init<>())
        .def("set_title", &HeaderBar::set_title)
        .def("get_title", &HeaderBar::get_title)
        .def("set_subtitle", &HeaderBar::set_subtitle)
        .def("get_subtitle", &HeaderBar::get_subtitle)
        .def("set_show_title_buttons", &HeaderBar::set_show_title_buttons)
        .def("get_show_title_buttons", &HeaderBar::get_show_title_buttons)
        .def("pack_start", &HeaderBar::pack_start)
        .def("pack_end", &HeaderBar::pack_end)
        .def("remove", &HeaderBar::remove);

    // ListBox
    py::class_<ListBox, Widget>(t, "ListBox")
        .def(py::init<>())
        .def("append", &ListBox::append)
        .def("remove", &ListBox::remove)
        .def("clear", &ListBox::clear)
        .def("select_row", &ListBox::select_row)
        .def("unselect_row", &ListBox::unselect_row)
        .def("get_selected_row", &ListBox::get_selected_row)
        .def("set_selection_mode", &ListBox::set_selection_mode);

    // ListBoxRow
    py::class_<ListBoxRow, Widget>(t, "ListBoxRow")
        .def(py::init([](const std::string& label) {
            auto row = new ListBoxRow();
            if (!label.empty()) {
                auto lbl = gtk_label_new(label.c_str());
                gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row->get_native()), lbl);
            }
            return row;
        }), py::arg("label") = "")
        .def("set_child", &ListBoxRow::set_child)
        .def("on_activate", &ListBoxRow::on_activate)
        .def("set_selected", &ListBoxRow::set_selected)
        .def("is_selected", &ListBoxRow::is_selected)
        .def("activate", &ListBoxRow::activate);

    // Window (layer shell)
    py::class_<Window, Widget>(t, "Window")
        .def(py::init([](std::string namespace_, int monitor, std::vector<std::string> anchor,
                         std::string exclusivity, std::string layer, std::string kb_mode,
                         bool popup, int margin_top, int margin_bottom,
                         int margin_left, int margin_right, bool dynamic_input_region) {
            WindowProperties props;
            props.namespace_ = namespace_;
            props.monitor = monitor;
            props.anchor = anchor;
            props.exclusivity = exclusivity;
            props.layer = layer;
            props.kb_mode = kb_mode;
            props.popup = popup;
            props.margin_top = margin_top;
            props.margin_bottom = margin_bottom;
            props.margin_left = margin_left;
            props.margin_right = margin_right;
            props.dynamic_input_region = dynamic_input_region;
            return new Window(props);
        }),
        py::arg("namespace"),
        py::arg("monitor") = 0,
        py::arg("anchor") = std::vector<std::string>{},
        py::arg("exclusivity") = "normal",
        py::arg("layer") = "top",
        py::arg("kb_mode") = "none",
        py::arg("popup") = false,
        py::arg("margin_top") = 0,
        py::arg("margin_bottom") = 0,
        py::arg("margin_left") = 0,
        py::arg("margin_right") = 0,
        py::arg("dynamic_input_region") = false)
        .def("set_child", &Window::set_child)
        .def("get_namespace", &Window::get_namespace)
        .def("set_monitor", &Window::set_monitor)
        .def("get_monitor", &Window::get_monitor)
        .def("set_layer", &Window::set_layer)
        .def("get_layer", &Window::get_layer)
        .def("set_kb_mode", &Window::set_kb_mode)
        .def("get_kb_mode", &Window::get_kb_mode)
        .def("set_exclusivity", &Window::set_exclusivity)
        .def("get_exclusivity", &Window::get_exclusivity)
        .def("set_popup", &Window::set_popup)
        .def("get_popup", &Window::get_popup)
        .def("set_input_width", &Window::set_input_width)
        .def("set_input_height", &Window::set_input_height)
        .def("get_input_width", &Window::get_input_width)
        .def("get_input_height", &Window::get_input_height)
        .def("set_anchor", &Window::set_anchor)
        .def("get_anchor", &Window::get_anchor)
        .def("set_margin", &Window::set_margin)
        .def("set_dynamic_input_region", &Window::set_dynamic_input_region)
        .def("get_dynamic_input_region", &Window::get_dynamic_input_region);

    // Panel (keeping backward compatibility - wraps Window)
    py::class_<Panel, Widget>(t, "Panel")
        .def(py::init([](std::string namespace_, std::vector<std::string> anchor, bool exclusive,
                        std::string layer, std::string kb_mode, bool popup, int width, int height,
                        int margin_top, int margin_bottom, int margin_left, int margin_right) {
            PanelProperties props;
            props.namespace_ = namespace_;
            props.anchor = anchor;
            props.exclusive = exclusive;
            props.layer = layer;
            props.kb_mode = kb_mode;
            props.popup = popup;
            props.width = width;
            props.height = height;
            props.margin_top = margin_top;
            props.margin_bottom = margin_bottom;
            props.margin_left = margin_left;
            props.margin_right = margin_right;
            return new Panel(props);
        }),
        py::arg("namespace") = "panel",
        py::arg("anchor") = std::vector<std::string>{},
        py::arg("exclusive") = true,
        py::arg("layer") = "top",
        py::arg("kb_mode") = "none",
        py::arg("popup") = false,
        py::arg("width") = -1,
        py::arg("height") = 40,
        py::arg("margin_top") = 0,
        py::arg("margin_bottom") = 0,
        py::arg("margin_left") = 0,
        py::arg("margin_right") = 0)
        .def("set_width", &Panel::set_width)
        .def("set_height", &Panel::set_height)
        .def("set_size", &Panel::set_size)
        .def("set_child", &Panel::set_child)
        .def("set_margin", &Panel::set_margin);

    // RegularWindow
    py::class_<RegularWindow, Widget>(t, "RegularWindow")
        .def(py::init<const std::string&, const std::string&>(),
             py::arg("namespace"), py::arg("title") = "")
        .def("set_title", &RegularWindow::set_title)
        .def("get_title", &RegularWindow::get_title)
        .def("get_namespace", &RegularWindow::get_namespace)
        .def("set_child", &RegularWindow::set_child)
        .def("set_titlebar", &RegularWindow::set_titlebar)
        .def("set_default_size", &RegularWindow::set_default_size);

    // RevealerWindow
    py::class_<RevealerWindow, Widget>(t, "RevealerWindow")
        .def(py::init<const std::string&, int, std::vector<std::string>, const std::string&>(),
             py::arg("namespace"), py::arg("monitor") = 0,
             py::arg("anchor") = std::vector<std::string>{},
             py::arg("layer") = "overlay")
        .def("set_child", &RevealerWindow::set_child)
        .def("reveal", &RevealerWindow::reveal)
        .def("hide_window", &RevealerWindow::hide_window)
        .def("toggle", &RevealerWindow::toggle)
        .def("is_visible", &RevealerWindow::is_visible)
        .def("get_namespace", &RevealerWindow::get_namespace);

    // PopoverMenu
    py::class_<PopoverMenu, Widget>(t, "PopoverMenu")
        .def(py::init<>())
        .def("add_child", &PopoverMenu::add_child, py::arg("child"), py::arg("id") = "")
        .def("set_parent", &PopoverMenu::set_parent)
        .def("set_pointing_to", &PopoverMenu::set_pointing_to)
        .def("popup", &PopoverMenu::popup)
        .def("popdown", &PopoverMenu::popdown);

    // helium.managers submodule
    py::module_ mgr = m.def_submodule("managers", "Helium managers");

    // WindowManager
    py::class_<WindowManager>(mgr, "WindowManager")
        .def_static("get_default", &WindowManager::get_default, py::return_value_policy::reference)
        .def("get_window", &WindowManager::get_window, py::return_value_policy::reference)
        .def("has_window", &WindowManager::has_window)
        .def("add_window", &WindowManager::add_window)
        .def("remove_window", &WindowManager::remove_window);

    // IconManager
    py::class_<IconManager>(mgr, "IconManager")
        .def_static("get_default", &IconManager::get_default, py::return_value_policy::reference)
        .def("add_icons", &IconManager::add_icons)
        .def("remove_icons", &IconManager::remove_icons)
        .def("get_added_icons", &IconManager::get_added_icons);

    // CssManager
    py::class_<CSS_MANAGER>(mgr, "CssManager")
        .def_static("load", [](const std::string& path) { CSS_MANAGER::load(path.c_str()); })
        .def_static("load_string", &CSS_MANAGER::load_string)
        .def_static("remove", &CSS_MANAGER::remove)
        .def_static("reload", &CSS_MANAGER::reload)
        .def_static("reset", &CSS_MANAGER::reset);

    // helium.services submodule
    py::module_ svc = m.def_submodule("services", "Helium services");

    // HyprlandService
    py::class_<HyprlandService> hypr_svc(svc, "HyprlandService");
    hypr_svc.def_static("get_default", &HyprlandService::get_default, py::return_value_policy::reference)
        .def("start_listening", &HyprlandService::start_listening)
        .def("stop_listening", &HyprlandService::stop_listening)
        .def("is_listening", &HyprlandService::is_listening)
        .def("connect_signal", &HyprlandService::connect_signal)
        .def("dispatch", &HyprlandService::dispatch)
        .def("send_command", &HyprlandService::send_command)
        .def("get_active_workspace_id", &HyprlandService::get_active_workspace_id)
        .def("get_active_window_class", &HyprlandService::get_active_window_class)
        .def("get_active_window_title", &HyprlandService::get_active_window_title);

    // AudioService
    py::class_<AudioService> audio_svc(svc, "AudioService");
    audio_svc.def_static("get_default", &AudioService::get_default, py::return_value_policy::reference)
        .def("get_volume", &AudioService::get_volume)
        .def("set_volume", &AudioService::set_volume)
        .def("get_muted", &AudioService::get_muted)
        .def("set_muted", &AudioService::set_muted)
        .def("toggle_muted", &AudioService::toggle_muted)
        .def("get_default_sink_name", &AudioService::get_default_sink_name)
        .def("get_default_sink_description", &AudioService::get_default_sink_description)
        .def("connect_signal", &AudioService::connect_signal)
        .def("start_polling", &AudioService::start_polling)
        .def("stop_polling", &AudioService::stop_polling);

    // Notification
    py::class_<Notification>(svc, "Notification")
        .def_readwrite("id", &Notification::id)
        .def_readwrite("app_name", &Notification::app_name)
        .def_readwrite("summary", &Notification::summary)
        .def_readwrite("body", &Notification::body)
        .def_readwrite("icon", &Notification::icon)
        .def_readwrite("urgency", &Notification::urgency)
        .def_readwrite("timestamp", &Notification::timestamp)
        .def_readwrite("category", &Notification::category)
        .def_readwrite("popup", &Notification::popup);

    // NotificationService
    py::class_<NotificationService> notif_svc(svc, "NotificationService");
    notif_svc.def_static("get_default", &NotificationService::get_default, py::return_value_policy::reference)
        .def("get_dnd", &NotificationService::get_dnd)
        .def("set_dnd", &NotificationService::set_dnd)
        .def("get_count", &NotificationService::get_count)
        .def("get_notifications", &NotificationService::get_notifications)
        .def("add_notification", &NotificationService::add_notification, py::arg("app_name"), py::arg("summary"), py::arg("body") = "", py::arg("icon") = "", py::arg("urgency") = 0, py::arg("category") = "")
        .def("connect_signal", &NotificationService::connect_signal);

    // BluetoothService
    py::class_<BluetoothService> bt_svc(svc, "BluetoothService");
    bt_svc.def_static("get_default", &BluetoothService::get_default, py::return_value_policy::reference)
        .def("is_bluetooth_on", &BluetoothService::is_bluetooth_on)
        .def("set_bluetooth_on", &BluetoothService::set_bluetooth_on)
        .def("is_connected", &BluetoothService::is_connected, py::arg("mac") = "")
        .def("get_devices", &BluetoothService::get_devices)
        .def("connect_signal", &BluetoothService::connect_signal);

    // NetworkService
    py::class_<NetworkService> net_svc(svc, "NetworkService");
    net_svc.def_static("get_default", &NetworkService::get_default, py::return_value_policy::reference)
        .def("get_ssid", &NetworkService::get_ssid)
        .def("get_strength", &NetworkService::get_strength)
        .def("is_connected", &NetworkService::is_connected)
        .def("get_icon_name", &NetworkService::get_icon_name)
        .def("connect_signal", &NetworkService::connect_signal);

    // MprisService
    py::class_<MprisService> mpris_svc(svc, "MprisService");
    mpris_svc.def_static("get_default", &MprisService::get_default, py::return_value_policy::reference)
        .def("get_artist", &MprisService::get_artist, py::arg("player") = "")
        .def("get_title", &MprisService::get_title, py::arg("player") = "")
        .def("get_status", &MprisService::get_status, py::arg("player") = "")
        .def("get_art_url", &MprisService::get_art_url, py::arg("player") = "")
        .def("get_players", &MprisService::get_players)
        .def("start_polling", &MprisService::start_polling, py::arg("interval_ms") = 2000)
        .def("stop_polling", &MprisService::stop_polling)
        .def("connect_signal", &MprisService::connect_signal);

    // BacklightService
    py::class_<BacklightService> backlight_svc(svc, "BacklightService");
    backlight_svc.def_static("get_default", &BacklightService::get_default, py::return_value_policy::reference)
        .def("get_brightness", &BacklightService::get_brightness)
        .def("set_brightness", &BacklightService::set_brightness)
        .def("get_max_brightness", &BacklightService::get_max_brightness)
        .def("get_brightness_percent", &BacklightService::get_brightness_percent);

    // WallpaperService
    py::class_<WallpaperService> wall_svc(svc, "WallpaperService");
    wall_svc.def_static("get_default", &WallpaperService::get_default, py::return_value_policy::reference)
        .def("set_wallpaper", &WallpaperService::set_wallpaper)
        .def("get_wallpaper", &WallpaperService::get_wallpaper);

    // PowerProfilesService
    py::class_<PowerProfilesService> pow_svc(svc, "PowerProfilesService");
    pow_svc.def_static("get_default", &PowerProfilesService::get_default, py::return_value_policy::reference)
        .def("get_profile", &PowerProfilesService::get_profile)
        .def("set_profile", &PowerProfilesService::set_profile)
        .def("get_profiles", &PowerProfilesService::get_profiles);

    // ApplicationService
    py::class_<ApplicationService> app_svc(svc, "ApplicationService");
    app_svc.def_static("get_default", &ApplicationService::get_default, py::return_value_policy::reference)
        .def("get_applications", &ApplicationService::get_applications)
        .def("launch", [](ApplicationService& self, const std::string& app_id) {
            self.launch(app_id);
        }, py::arg("app_id"));

    // SystemTrayService
    py::class_<SystemTrayService> tray_svc(svc, "SystemTrayService");
    tray_svc.def_static("get_default", &SystemTrayService::get_default, py::return_value_policy::reference)
        .def("is_available", &SystemTrayService::is_available)
        .def("get_items", &SystemTrayService::get_items)
        .def("connect_signal", &SystemTrayService::connect_signal)
        .def("start_polling", &SystemTrayService::start_polling, py::arg("interval_ms") = 5000)
        .def("stop_polling", &SystemTrayService::stop_polling);
}
