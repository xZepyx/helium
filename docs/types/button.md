# Button

A clickable button with a label.

```python
from helium.types import Button

btn = Button("Click Me")
btn.on_click(lambda: print("clicked!"))

btn.set_label("New Label")
text = btn.get_label()
```

## Constructor

`Button(label: str = "")`

## Methods

| Method | What it does |
|---|---|
| `on_click(callback)` | Fires when clicked |
| `set_label(text)` | Changes the label text |
| `get_label()` | Returns the label text |
