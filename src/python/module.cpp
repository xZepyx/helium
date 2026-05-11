#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h> // Critical for passing Python functions as std::function
#include <gtk/gtk.h>

#include "../types/widget.hpp"
#include "../types/label.hpp"
#include "../types/panel.hpp"
#include "../types/box.hpp"
#include "../types/button.hpp"
#include "../types/centerbox.hpp"
#include "../types/image.hpp"
#include "../types/entry.hpp"
#include "../managers/cssmanager.hpp"
#include "../functions/poll.hpp"

namespace py = pybind11;

// I put all the shit I could process here.
// It was real pain in the ass to read the documentation.

PYBIND11_MODULE(helium, m) {
    m.doc() = "Helium shell framework";

    m.def("init", []() { gtk_init(); });
    m.def("run", []() {
        GMainLoop* loop = g_main_loop_new(nullptr, false);
        g_main_loop_run(loop);
    });
    m.def("load_css", [](const std::string& path) {
        CSS_MANAGER::load(path.c_str());
    }, py::arg("path"));

    // helium.functions submodule
    py::module_ f = m.def_submodule("functions", "Helium functional utilities");
    f.def("Poll", &create_poll, py::arg("ms"), py::arg("callback"));

    // helium.types submodule
    py::module_ t = m.def_submodule("types", "Helium widget types.");

    py::class_<Widget>(t, "Widget")
        .def("show", &Widget::show)
        .def("add_css_class", &Widget::add_css_class)
        .def("remove_css_class", &Widget::remove_css_class)
        .def("connect", &Widget::connect_signal, py::arg("signal_name"), py::arg("callback"))
        .def("set_hexpand", [](Widget& w, bool expand) { gtk_widget_set_hexpand(w.get_native(), expand); })
        .def("set_vexpand", [](Widget& w, bool expand) { gtk_widget_set_vexpand(w.get_native(), expand); });

    py::class_<Label, Widget>(t, "Label")
        .def(py::init<const std::string&>(), py::arg("label") = "")
        .def("set_label", &Label::set_label)
        .def("get_label", &Label::get_label);

    py::class_<Button, Widget>(t, "Button")
        .def(py::init([](std::string label) {
            ButtonProperties props;
            props.label = label;
            return new Button(props);
        }), py::arg("label") = "")
        .def("on_click", &Button::on_click, py::arg("callback"));

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
        .def("add", &Box::add);

    py::class_<CenterBox, Widget>(t, "CenterBox")
        .def(py::init<>())
        .def("set_start", &CenterBox::set_start)
        .def("set_center", &CenterBox::set_center)
        .def("set_end", &CenterBox::set_end);

    py::class_<Image, Widget>(t, "Image")
        .def_static("from_icon", &Image::from_icon, py::arg("name"))
        .def_static("from_file", &Image::from_file, py::arg("path"));

    py::class_<Entry, Widget>(t, "Entry")
        .def(py::init<>())
        .def("set_text", &Entry::set_text)
        .def("get_text", &Entry::get_text)
        .def("on_activate", &Entry::on_activate);

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
}