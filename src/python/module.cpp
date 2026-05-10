#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <gtk/gtk.h>

#include "../types/widget.hpp"
#include "../types/label.hpp"
#include "../types/panel.hpp"
#include "../types/box.hpp"
#include "../types/button.hpp"

#include "../managers/cssmanager.hpp"

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

    py::class_<Widget>(m, "Widget")
        .def("show", &Widget::show)
        .def("add_css_class", &Widget::add_css_class);

    py::class_<Label, Widget>(m, "Label")
        .def(py::init<const std::string&>(), py::arg("label") = "")
        .def("set_label", &Label::set_label)
        .def("get_label", &Label::get_label);

    py::class_<Button, Widget>(m, "Button")
            .def(py::init([](std::string label) {
                ButtonProperties props;
                props.label = label;
                return new Button(props);
            }), py::arg("label") = "");

    py::class_<Box, Widget>(m, "Box")
        .def(py::init([](std::string orientation, int spacing, std::vector<Widget*> children) {
            BoxProperties props;
            props.orientation = orientation;
            props.spacing = spacing;
            props.children = children;
            return new Box(props);
        }), 
        py::arg("orientation") = "horizontal", 
        py::arg("spacing") = 0, 
        py::arg("children") = std::vector<Widget*>{})
        .def("add", &Box::add);

    py::class_<Panel, Widget>(m, "Panel")
        .def(py::init([](std::string namespace_, std::vector<std::string> anchor, bool exclusive, std::string layer, std::string kb_mode, bool popup, int width, int height) {
            PanelProperties props;

            props.namespace_ = namespace_;
            props.anchor = anchor;
            props.exclusive = exclusive;
            props.layer = layer;
            props.kb_mode = kb_mode;
            props.popup = popup;
            props.width = width;
            props.height = height;

            return new Panel(props);
        }),

        py::arg("namespace") = "panel",
        py::arg("anchor") = std::vector<std::string>{},
        py::arg("exclusive") = true,
        py::arg("layer") = "top",
        py::arg("kb_mode") = "none",
        py::arg("popup") = false,
        py::arg("width") = -1,
        py::arg("height") = 40
    )

    .def("set_width", &Panel::set_width)
    .def("set_height", &Panel::set_height)
    .def("set_size", &Panel::set_size)
    .def("set_child", &Panel::set_child);
}
