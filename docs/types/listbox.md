# ListBox

A vertical list of selectable rows.

```python
from helium.types import ListBox, ListBoxRow, Label

listbox = ListBox()

row1 = ListBoxRow()
row1.set_child(Label("Item 1"))

row2 = ListBoxRow()
row2.set_child(Label("Item 2"))

listbox.append(row1)
listbox.append(row2)

listbox.select_row(row1)
selected = listbox.get_selected_row()
listbox.set_selection_mode("single")
listbox.clear()
```

## Constructor

`ListBox()`

## Methods

| Method | What it does |
|---|---|
| `append(row)` | Appends a row |
| `remove(row)` | Removes a row |
| `clear()` | Removes all rows |
| `select_row(row)` | Selects a row |
| `unselect_row(row)` | Unselects a row |
| `get_selected_row()` | Returns selected row |
| `set_selection_mode(mode)` | `"none"`, `"single"`, `"browse"`, `"multiple"` |
