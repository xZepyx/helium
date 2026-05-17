# Label

A text label. Supports Pango markup and various display options.

```python
from helium.types import Label

label = Label("hello world")
label.set_label("new text")
text = label.get_label()

label.set_use_markup(True)
label.set_label("<b>bold</b> text")

label.set_wrap(True)
label.set_ellipsize("end")
label.set_max_width_chars(30)
label.set_xalign(0.5)
label.set_justify("center")
```

## Constructor

`Label(label: str = "")`

## Methods

| Method | What it does |
|---|---|
| `set_label(text)` | Changes the text |
| `get_label()` | Returns the text |
| `set_use_markup(v)` | Enables Pango markup |
| `set_wrap(v)` | Wraps at word boundaries |
| `set_justify(v)` | `"left"`, `"center"`, `"right"`, `"fill"` |
| `set_ellipsize(v)` | `"none"`, `"start"`, `"middle"`, `"end"` |
| `set_max_width_chars(v)` | Max width in characters |
| `set_xalign(v)` | Horizontal alignment (0.0–1.0) |
