# MaterialSymbol

A label that displays a Material Symbols icon. Requires the Material Symbols font to be installed.

```python
from helium.types import MaterialSymbol

icon = MaterialSymbol("home", size=24, fill=False)
icon.set_label("settings")   # switch icon
```

## Constructor

`MaterialSymbol(symbol: str, size: float = 24.0, fill: bool = False)`

Extends `Label` so all Label methods are available.

## Methods

| Method | What it does |
|---|---|
| (inherits Label) | `set_label`, `get_label`, etc. |
