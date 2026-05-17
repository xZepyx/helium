# DropDown

A dropdown/combobox selector.

```python
from helium.types import DropDown

dd = DropDown(items=["A", "B", "C"], selected=0)

def on_selected(index):
    print(f"selected: {dd.get_selected_item()}")

dd.on_selected(on_selected)
dd.set_selected(2)
index = dd.get_selected()
item = dd.get_selected_item()
```

## Constructor

`DropDown(items: list = [], selected: int = 0)`

## Methods

| Method | What it does |
|---|---|
| `set_items(items)` | Sets the item list |
| `get_items()` | Returns the items |
| `set_selected(index)` | Selects by index |
| `get_selected()` | Returns selected index |
| `get_selected_item()` | Returns selected text |
| `on_selected(callback)` | Fires on selection |
