# RegularWindow

A standard GTK window (not a layer-shell window).

```python
from helium.types import RegularWindow

win = RegularWindow("my_app", title="My Application")
win.set_default_size(800, 600)
win.set_titlebar(my_header_bar)
win.set_child(my_content)
win.show()
```

## Constructor

`RegularWindow(namespace: str, title: str = "")`

## Methods

| Method | What it does |
|---|---|
| `set_title(v)` | Sets the title |
| `get_title()` | Returns the title |
| `get_namespace()` | Returns namespace |
| `set_child(child)` | Sets the child |
| `set_titlebar(tb)` | Sets custom titlebar |
| `set_default_size(w, h)` | Sets default size |
