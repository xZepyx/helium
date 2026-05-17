# Icon

A themed icon with configurable size.

```python
from helium.types import Icon

icon = Icon(pixel_size=24)
icon.set_icon_name("network-wireless-symbolic")
name = icon.get_icon_name()
icon.set_pixel_size(32)
size = icon.get_pixel_size()
```

Factory methods:

```python
icon = Icon.from_icon_name("network-wireless-symbolic", size=24)
icon = Icon.from_file("/path/to/icon.png", size=24)
```

## Constructor

`Icon(pixel_size: int = -1)`

## Methods

| Method | What it does |
|---|---|
| `set_icon_name(name)` | Sets from theme |
| `get_icon_name()` | Returns icon name |
| `set_from_file(path)` | Loads from file |
| `set_pixel_size(size)` | Sets pixel size |
| `get_pixel_size()` | Returns pixel size |
| `from_icon_name(name, size=-1)` | Creates from theme |
| `from_file(path, size=-1)` | Creates from file |
