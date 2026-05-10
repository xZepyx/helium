#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <gtk/gtk.h>

#include "../types/widget.hpp"
#include "../types/label.hpp"
#include "../types/panel.hpp"

namespace py = pybind11;

// I put all the shit I could process here.
// It was real pain in the ass to read the documenation.

PYBIND11_MODULE(helium, m) {
    m.doc() = "Helium - A gtk4 layer shell framework";

    m.def("init", []() {
        gtk_init();
    });

    m.def("run", []() {
        GMainLoop* loop =
            g_main_loop_new(
                nullptr,
                false
            );

        g_main_loop_run(loop);
    });
    py::class_<Widget>(m, "Widget")
        .def("show", &Widget::show)
        .def("add_css_class", &Widget::add_css_class);

    py::class_<Label, Widget>(m, "Label")
        .def(py::init<const std::string&>())
        .def("set_label", &Label::set_label)
        .def("get_label", &Label::get_label);

    py::class_<PanelProperties>(m, "PanelProperties")
        .def(py::init<>())

        .def_readwrite("namespace_", &PanelProperties::namespace_)
        .def_readwrite("monitor", &PanelProperties::monitor)
        .def_readwrite("anchor", &PanelProperties::anchor)
        .def_readwrite("exclusive", &PanelProperties::exclusive)
        .def_readwrite("layer", &PanelProperties::layer)
        .def_readwrite("kb_mode", &PanelProperties::kb_mode)
        .def_readwrite("popup", &PanelProperties::popup)
        .def_readwrite("width", &PanelProperties::width)
        .def_readwrite("height", &PanelProperties::height);

    py::class_<Panel, Widget>(m, "Panel")
        .def(py::init<const PanelProperties&>())
        .def("set_width", &Panel::set_width)
        .def("set_height", &Panel::set_height)
        .def("set_size", &Panel::set_size)
        .def("set_child", &Panel::set_child);
}