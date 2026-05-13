#pragma once
#include "label.hpp"
#include <string>

class MaterialSymbol : public Label {
public:
    MaterialSymbol(std::string symbol, float size = 24.0, bool fill = false) : Label(symbol) {
        // Set the font family to Material Symbols
        // Ensure "Material Symbols Outlined" is installed on your Arch system
        add_css_class("material-symbol-root");
        
        std::string variations = "'FILL' " + std::string(fill ? "1" : "0");
        
        // Apply inline styles to avoid external CSS requirements
        GtkWidget* native = get_native();
        std::string style = "font-family: 'Material Symbols Outlined'; "
                            "font-size: " + std::to_string(size) + "px; "
                            "font-variation-settings: " + variations + ";";
        
        // In GTK4, we usually apply this via a CSS provider or attributes
        // For a 'type', setting the Pango attributes is most reliable
        PangoAttrList* attrs = pango_attr_list_new();
        pango_attr_list_insert(attrs, pango_attr_family_new("Material Symbols Outlined"));
        pango_attr_list_insert(attrs, pango_attr_size_new(size * PANGO_SCALE));
        
        gtk_label_set_attributes(GTK_LABEL(native), attrs);
        pango_attr_list_unref(attrs);
    }
};