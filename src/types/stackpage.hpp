#pragma once

#include <string>

#include "widget.hpp"

struct StackPage {
    std::string title;
    Widget* child = nullptr;
};
