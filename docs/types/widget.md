# Widget

Base class for all visual elements. Everything inherits from this.

## Methods

| Method | What it does |
|---|---|
| `show()` | Makes it visible |
| `hide()` | Hides it |
| `get_visible()` | Returns visibility |
| `add_css_class(c)` | Adds a CSS class |
| `remove_css_class(c)` | Removes a CSS class |
| `set_style(css)` | Applies inline CSS |
| `connect(signal, cb)` | Connects a GTK signal |
| `set_hexpand(v)` | Expand horizontally |
| `get_hexpand()` | Check horizontal expand |
| `set_vexpand(v)` | Expand vertically |
| `get_vexpand()` | Check vertical expand |
| `set_halign(v)` | `"fill"`, `"start"`, `"center"`, `"end"` |
| `set_valign(v)` | `"fill"`, `"start"`, `"center"`, `"end"` |
| `set_size_request(w, h)` | Minimum width/height |
| `set_name(v)` | Widget name for CSS selectors |
| `get_name()` | Returns widget name |
| `set_sensitive(v)` | Enable/disable interaction |
| `get_sensitive()` | Returns sensitive state |

## Example

```python
widget.show()
widget.hide()
widget.add_css_class("my-class")
widget.set_style("color: white; background: black;")
widget.set_size_request(100, 30)
```
