# Entry

A single-line text input.

```python
from helium.types import Entry

entry = Entry()
entry.set_text("default text")
text = entry.get_text()

entry.on_activate(lambda: print("pressed enter"))
entry.on_change(lambda: print("text changed"))
entry.set_placeholder_text("type something...")
```

## Constructor

`Entry()`

## Methods

| Method | What it does |
|---|---|
| `set_text(text)` | Sets the text |
| `get_text()` | Returns the text |
| `on_activate(callback)` | Fires on Enter |
| `on_change(callback)` | Fires on every change |
| `set_placeholder_text(text)` | Ghost text when empty |
