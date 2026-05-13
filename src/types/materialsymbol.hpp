#pragma once
#include "label.hpp"
#include <string>

class MaterialSymbol : public Label {
public:
    MaterialSymbol(std::string symbol, float size = 24.0, bool fill = false) : Label(symbol) {
        add_css_class("material-symbol-root");
        
        std::string variations = fill ? "1" : "0";
        
        std::string icon_style = 
            "font-family: 'Material Symbols Outlined'; "
            "font-size: " + std::to_string(size) + "px; "
            "font-variation-settings: 'FILL' " + variations + ";";
        
        // 3. Call the base Widget method we just created!
        set_style(icon_style);
    }
};