# Picture

Displays an image file. Meant for larger images and photos (not small icons).

```python
from helium.types import Picture

pic = Picture("path/to/image.jpg")
pic.set_filename("path/to/other.jpg")
name = pic.get_filename()

pic.set_keep_aspect_ratio(True)
pic.set_can_shrink(False)
```

## Constructor

`Picture(file_path: str = "")`

## Methods

| Method | What it does |
|---|---|
| `set_filename(path)` | Loads an image from file |
| `get_filename()` | Returns the filename |
| `set_keep_aspect_ratio(v)` | Preserve aspect ratio |
| `get_keep_aspect_ratio()` | Returns aspect ratio setting |
| `set_can_shrink(v)` | Allow shrinking |
| `get_can_shrink()` | Returns shrinkability |
