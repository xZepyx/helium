# ArrowButton

A button containing an Arrow and optional label.

```python
from helium.types import Arrow, ArrowButton

arrow = Arrow()
btn = ArrowButton(arrow, label="expand")
btn.toggle()
```

## Constructor

`ArrowButton(arrow: Arrow, label: str = "")`

## Methods

| Method | What it does |
|---|---|
| `toggle()` | Toggles arrow rotation |
