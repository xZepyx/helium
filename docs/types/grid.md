# Grid

A table-like layout where children are placed by coordinates.

```python
from helium.types import Grid, Label

grid = Grid()
grid.attach(Label("Name"), column=0, row=0)
grid.attach(Label("Value"), column=1, row=0)
grid.attach(Label("hello"), column=0, row=1, width=2)
```

## Constructor

`Grid(column_num: int = 0, row_num: int = 0)`

## Methods

| Method | What it does |
|---|---|
| `attach(child, column, row, width=1, height=1)` | Places at position |
| `remove(child)` | Removes a child |
| `set_column_homogeneous(v)` | All columns same width |
| `set_row_homogeneous(v)` | All rows same height |
| `set_column_spacing(v)` | Space between columns |
| `set_row_spacing(v)` | Space between rows |
