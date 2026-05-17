# Managers

Managers are global registries that keep track of windows, loaded CSS, and icon theme paths.

## WindowManager

Keeps a registry of all windows by their namespace. Lets you look up and manage windows from anywhere in your code.

```python
from helium.managers import WindowManager

wm = WindowManager.get_default()

if wm.has_window("my_bar"):
    win = wm.get_window("my_bar")
    print(win.get_namespace())
```

Windows are automatically registered when created and unregistered when destroyed. You typically don't need to call `add_window` or `remove_window` yourself.

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `get_window(namespace)` | Looks up a window by namespace |
| `has_window(namespace)` | Checks if a window exists |
| `add_window(namespace, window)` | Registers a window |
| `remove_window(namespace)` | Unregisters a window |

## IconManager

Manages additional icon search paths. If you have custom icons in a directory, add them here.

```python
from helium.managers import IconManager

im = IconManager.get_default()
im.add_icons("/home/user/.local/share/icons")
im.remove_icons("/home/user/.local/share/icons")

paths = im.get_added_icons()
```

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `add_icons(path)` | Adds an icon search path |
| `remove_icons(path)` | Removes a search path |
| `get_added_icons()` | Returns all added paths |

## CssManager

Controls which CSS stylesheets are loaded. All methods are static.

```python
from helium.managers import CssManager

CssManager.load("path/to/style.css")
CssManager.load_string("* { color: white; }", priority="application")
CssManager.remove("path/to/style.css")
CssManager.reload()   # hot-reload all files
CssManager.reset()    # remove everything
```

Priority levels: `"application"` (default), `"user"`, `"theme"`, `"fallback"`.

The CSS file paths are auto-watched via `GFileMonitor`, so calling `reload()` will pick up any changes you've made to the files.

| Method | What it does |
|---|---|
| `load(path)` | Loads a CSS file |
| `load_string(css, priority="application")` | Loads CSS from a string |
| `remove(identifier)` | Removes a CSS provider by path |
| `reload()` | Hot-reloads all file-based CSS |
| `reset()` | Removes all CSS providers |
