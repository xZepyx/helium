# HeaderBar

A window title bar with title, subtitle, and widget slots.

```python
from helium.types import HeaderBar, Button

bar = HeaderBar()
bar.set_title("My App")
bar.set_subtitle("v1.0")
bar.pack_start(Button("menu"))
bar.pack_end(Button("close"))
bar.set_show_title_buttons(True)
```

## Constructor

`HeaderBar()`

## Methods

| Method | What it does |
|---|---|
| `set_title(v)` | Sets the title |
| `get_title()` | Returns the title |
| `set_subtitle(v)` | Sets the subtitle |
| `get_subtitle()` | Returns the subtitle |
| `set_show_title_buttons(v)` | Shows/hides window buttons |
| `get_show_title_buttons()` | Returns button visibility |
| `pack_start(child)` | Packs left |
| `pack_end(child)` | Packs right |
| `remove(child)` | Removes a packed child |
