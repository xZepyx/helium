# ApplicationService

Discovers and launches installed applications by parsing `.desktop` files.

```python
apps = None
try:
    apps = helium.services.ApplicationService.get_default()
except Exception:
    pass

if apps:
    all_apps = apps.get_applications()
    for app in all_apps:
        print(app.name, app.exec)

    apps.launch("firefox")
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `get_applications()` | Returns all installed apps |
| `launch(app_id)` | Launches an app by desktop ID |
