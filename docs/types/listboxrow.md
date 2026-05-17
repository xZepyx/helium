# ListBoxRow

An individual row in a ListBox.

```python
from helium.types import ListBoxRow, Label

row = ListBoxRow()
row.set_child(Label("Row content"))
row.on_activate(lambda: print("activated"))
row.activate()
row.set_selected(True)
selected = row.is_selected()
```

## Constructor

`ListBoxRow(label: str = "")`

## Methods

| Method | What it does |
|---|---|
| `set_child(child)` | Sets the child widget |
| `set_selected(v)` | Sets visual selected state |
| `is_selected()` | Returns selected state |
| `on_activate(callback)` | Fires when activated |
| `activate()` | Programmatically triggers activate |
